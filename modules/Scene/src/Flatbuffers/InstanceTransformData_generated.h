// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_INSTANCETRANSFORMDATA_DEEPSEASCENE_H_
#define FLATBUFFERS_GENERATED_INSTANCETRANSFORMDATA_DEEPSEASCENE_H_

#include "flatbuffers/flatbuffers.h"

namespace DeepSeaScene {

struct InstanceTransformData;

struct InstanceTransformData FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_VARIABLEGROUPDESCNAME = 4
  };
  const flatbuffers::String *variableGroupDescName() const {
    return GetPointer<const flatbuffers::String *>(VT_VARIABLEGROUPDESCNAME);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_VARIABLEGROUPDESCNAME) &&
           verifier.VerifyString(variableGroupDescName()) &&
           verifier.EndTable();
  }
};

struct InstanceTransformDataBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_variableGroupDescName(flatbuffers::Offset<flatbuffers::String> variableGroupDescName) {
    fbb_.AddOffset(InstanceTransformData::VT_VARIABLEGROUPDESCNAME, variableGroupDescName);
  }
  explicit InstanceTransformDataBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  InstanceTransformDataBuilder &operator=(const InstanceTransformDataBuilder &);
  flatbuffers::Offset<InstanceTransformData> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<InstanceTransformData>(end);
    fbb_.Required(o, InstanceTransformData::VT_VARIABLEGROUPDESCNAME);
    return o;
  }
};

inline flatbuffers::Offset<InstanceTransformData> CreateInstanceTransformData(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> variableGroupDescName = 0) {
  InstanceTransformDataBuilder builder_(_fbb);
  builder_.add_variableGroupDescName(variableGroupDescName);
  return builder_.Finish();
}

inline flatbuffers::Offset<InstanceTransformData> CreateInstanceTransformDataDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *variableGroupDescName = nullptr) {
  auto variableGroupDescName__ = variableGroupDescName ? _fbb.CreateString(variableGroupDescName) : 0;
  return DeepSeaScene::CreateInstanceTransformData(
      _fbb,
      variableGroupDescName__);
}

inline const DeepSeaScene::InstanceTransformData *GetInstanceTransformData(const void *buf) {
  return flatbuffers::GetRoot<DeepSeaScene::InstanceTransformData>(buf);
}

inline const DeepSeaScene::InstanceTransformData *GetSizePrefixedInstanceTransformData(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<DeepSeaScene::InstanceTransformData>(buf);
}

inline bool VerifyInstanceTransformDataBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<DeepSeaScene::InstanceTransformData>(nullptr);
}

inline bool VerifySizePrefixedInstanceTransformDataBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<DeepSeaScene::InstanceTransformData>(nullptr);
}

inline void FinishInstanceTransformDataBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::InstanceTransformData> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedInstanceTransformDataBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<DeepSeaScene::InstanceTransformData> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace DeepSeaScene

#endif  // FLATBUFFERS_GENERATED_INSTANCETRANSFORMDATA_DEEPSEASCENE_H_
