# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class TextureInfo(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsTextureInfo(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = TextureInfo()
        x.Init(buf, n + offset)
        return x

    # TextureInfo
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # TextureInfo
    def Format(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # TextureInfo
    def Decoration(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # TextureInfo
    def Dimension(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # TextureInfo
    def Width(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # TextureInfo
    def Height(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # TextureInfo
    def Depth(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # TextureInfo
    def MipLevels(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

def TextureInfoStart(builder): builder.StartObject(7)
def TextureInfoAddFormat(builder, format): builder.PrependUint8Slot(0, format, 0)
def TextureInfoAddDecoration(builder, decoration): builder.PrependUint8Slot(1, decoration, 0)
def TextureInfoAddDimension(builder, dimension): builder.PrependUint8Slot(2, dimension, 0)
def TextureInfoAddWidth(builder, width): builder.PrependUint32Slot(3, width, 0)
def TextureInfoAddHeight(builder, height): builder.PrependUint32Slot(4, height, 0)
def TextureInfoAddDepth(builder, depth): builder.PrependUint32Slot(5, depth, 0)
def TextureInfoAddMipLevels(builder, mipLevels): builder.PrependUint8Slot(6, mipLevels, 0)
def TextureInfoEnd(builder): return builder.EndObject()
