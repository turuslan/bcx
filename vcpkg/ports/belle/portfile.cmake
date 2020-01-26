
vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO Asmozan/belle
  REF 2a9f2656b07a974b4b583a6e46c383244a19755f
  SHA512 c570c700c30873dcfe6a8c7f39b7247c667a8299d4768a2f0d594ae790b610c6536e4dc3c74e7796e7c6e2992709cfb0e13c4ef7a4e5abf2fe378e94dc731f78
  HEAD_REF master
  )

file(INSTALL ${SOURCE_PATH}/include/belle.hh DESTINATION ${CURRENT_PACKAGES_DIR}/include)

file(INSTALL ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/belle RENAME copyright)
