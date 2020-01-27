#include <ed25519/ed25519.h>
#include <grpcpp/grpcpp.h>
#include <boost/asio/io_context.hpp>

#include "db/db.hpp"
#include "format/format.hpp"
#include "gen/pb/endpoint.grpc.pb.h"
#include "sync/sync.hpp"

namespace bcx {
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

  void syncBlock(boost::asio::io_context &io,
                 const iroha::protocol::Block &block) {
    io.post([block]() {
      logger::info("sync block {}", format::blockHeight(block));
      db::addBlock(block);
    });
  }

  void runSync(boost::asio::io_context &io) {
    IrohaApi api{*config.iroha};
    logger::info("sync start");
    auto stream = api.fetchCommits();
    auto height = db::blockCount() + 1;
    iroha::protocol::Block block;
    while (api.getBlock(block, height)) {
      syncBlock(io, block);
      ++height;
    }
    io.post([block]() { logger::info("sync wait for new blocks"); });
    iroha::protocol::BlockQueryResponse res;
    while (stream.first->Read(&res)) {
      if (res.has_block_error_response()) {
        fatal("FetchCommits error {}", res.block_error_response().message());
      }
      auto &res_block = res.block_response().block();
      if (height <= format::blockHeight(res_block)) {
        syncBlock(io, res_block);
      }
    }
    auto status = stream.first->Finish();
    if (!status.ok()) {
      fatal("GRPC error {}", status.error_message());
    }
    logger::info("sync stop");
  }
}  // namespace bcx
