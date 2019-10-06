# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers

class Texture(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsTexture(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Texture()
        x.Init(buf, n + offset)
        return x

    # Texture
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Texture
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # Texture
    def Usage(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # Texture
    def MemoryHints(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # Texture
    def Path(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # Texture
    def TextureInfo(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            x = self._tab.Indirect(o + self._tab.Pos)
            from .TextureInfo import TextureInfo
            obj = TextureInfo()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

def TextureStart(builder): builder.StartObject(5)
def TextureAddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def TextureAddUsage(builder, usage): builder.PrependUint32Slot(1, usage, 0)
def TextureAddMemoryHints(builder, memoryHints): builder.PrependUint32Slot(2, memoryHints, 0)
def TextureAddPath(builder, path): builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(path), 0)
def TextureAddTextureInfo(builder, textureInfo): builder.PrependUOffsetTRelativeSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(textureInfo), 0)
def TextureEnd(builder): return builder.EndObject()