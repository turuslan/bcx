#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <ed25519/ed25519.h>
#include <grpcpp/grpcpp.h>
#include <queue>
#include <thread>

#include "db/db.hpp"
#include "format/format.hpp"
#include "gen/pb/endpoint.grpc.pb.h"
#include "sync/sync.hpp"

namespace bcx {
  constexpr auto kGetBlockThreads = 10u;
  constexpr auto kGetBlockQueue = 30u;

  struct IrohaApi {
    IrohaApi(const Config::Iroha &config)
        : service{grpc::CreateChannel(config.host,
                                      grpc::InsecureChannelCredentials())},
          account{config.account} {
      std::copy(config.private_key.begin(),
                config.private_key.end(),
                std::begin(ed_private_key.data));
      ed25519_derive_public_key(&ed_private_key, &ed_public_key);
      checkAccount();
    }

    void checkAccount() {
      iroha::protocol::QueryResponse res;
      getBlock(res, 1);
      if (res.has_error_response()) {
        auto code = res.error_response().error_code();
        if (code == 3) {
          fatal("Invalid account or key");
        } else if (code == 2) {
          fatal("Account doesn't have get_blocks permission");
        }
      }
    }

    bool getBlock(iroha::protocol::Block &block, size_t height) {
      iroha::protocol::QueryResponse res;
      getBlock(res, height);
      if (res.has_error_response()) {
        return false;
      }
      block = res.block_response().block();
      return true;
    }

    void getBlock(iroha::protocol::QueryResponse &res, size_t height) {
      iroha::protocol::Query query;
      auto payload = query.mutable_payload();
      payload->mutable_get_block()->set_height(height);
      setMeta(*payload->mutable_meta());
      sign(*query.mutable_signature(), *payload);
      find(res, query);
    }

    void find(iroha::protocol::QueryResponse &res,
              iroha::protocol::Query &query) {
      grpc::ClientContext ctx;
      auto status = service.Find(&ctx, query, &res);
      if (!status.ok()) {
        fatal("GRPC error {}", status.error_message());
      }
    }

    auto fetchCommits() {
      iroha::protocol::BlocksQuery query;
      auto meta = query.mutable_meta();
      setMeta(*meta);
      sign(*query.mutable_signature(), *meta);

      auto ctx = std::make_shared<grpc::ClientContext>();
      return std::make_pair(service.FetchCommits(ctx.get(), query), ctx);
    }

    void sign(iroha::protocol::Signature &signature,
              const google::protobuf::MessageLite &payload) {
      auto hash = format::sha256(payload.SerializeAsString());
      signature_t ed_signature;
      ed25519_sign(&ed_signature,
                   hash.data(),
                   hash.size(),
                   &ed_public_key,
                   &ed_private_key);
      signature.set_public_key(format::hex(ed_public_key.data));
      signature.set_signature(format::hex(ed_signature.data));
    }

    void setMeta(iroha::protocol::QueryPayloadMeta &meta) {
      meta.set_creator_account_id(account);
      meta.set_created_time(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch())
              .count());
      meta.set_query_counter(1);
    }

    iroha::protocol::QueryService_v1::Stub service;
    std::string account;
    private_key_t ed_private_key;
    public_key_t ed_public_key;
  };

  struct BlockHeightLess {
    auto operator()(const iroha::protocol::Block &lhs, const iroha::protocol::Block &rhs) const {
      return format::blockHeight(lhs) > format::blockHeight(rhs);
    }
  };

  void runSync(boost::asio::io_context &io) {
    IrohaApi api{*config.iroha};
    auto last_height = db::blockCount();
    iroha::protocol::Block last_block;
    if (last_height != 0) {
      if (!api.getBlock(last_block, last_height) || format::blockHash(last_block) != db::block_hash[last_height - 1]) {
        db::drop();
        fatal("Last cached block differs, invalidating block cache");
      }
    }
    auto next_height = last_height + 1;
    logger::info("Sync start");

    std::priority_queue<iroha::protocol::Block, std::vector<iroha::protocol::Block>, BlockHeightLess> q;
    auto postBlock = [&](iroha::protocol::Block &&block) {
      io.post([&, block=std::move(block)]() {
        q.push(block);
        while (!q.empty()) {
          auto height = format::blockHeight(q.top());
          if (height > next_height) {
            break;
          }
          if (height == next_height) {
            logger::info("Sync block {}", format::blockHeight(q.top()));
            db::addBlock(q.top());
            ++next_height;
          }
          q.pop();
        }
      });
    };

    boost::asio::io_context wio;
    boost::asio::executor_work_guard wguard{wio.get_executor()};
    std::vector<std::thread> wthreads;
    for (auto i = 0u; i < kGetBlockThreads; ++i) {
      wthreads.emplace_back([&]() { wio.run(); });
    }
    auto qheight = next_height;
    auto qheight_max = std::numeric_limits<size_t>::max();
    auto stream = api.fetchCommits();
    boost::asio::io_context qio;
    boost::asio::executor_work_guard qguard{qio.get_executor()};
    auto qjob = [&](const auto &qjob2) -> void {
      if (qheight >= qheight_max) {
        qio.stop();
      } else {
        wio.post([&, qheight]() {
          if (qheight < qheight_max) {
            iroha::protocol::Block block;
            if (api.getBlock(block, qheight)) {
              postBlock(std::move(block));
            } else {
              qheight_max = std::min(qheight_max, qheight);
            }
            qio.post([&]() { qjob2(qjob2); });
          }
        });
        ++qheight;
      }
    };
    for (auto i = 0; i < kGetBlockQueue; ++i) {
      qjob(qjob);
    }
    qio.run();
    wguard.reset();
    for (auto &thread : wthreads) {
      thread.join();
    }
    io.post([]() { logger::info("Sync wait for new blocks"); });
    iroha::protocol::BlockQueryResponse res;
    while (stream.first->Read(&res)) {
      if (res.has_block_error_response()) {
        fatal("FetchCommits error {}", res.block_error_response().message());
      }
      postBlock(std::move(*res.mutable_block_response()->mutable_block()));
    }
    auto status = stream.first->Finish();
    if (!status.ok()) {
      fatal("GRPC error {}", status.error_message());
    }
    logger::info("Sync stop");
  }
}  // namespace bcx
