# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers

class VertexFormat(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsVertexFormat(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = VertexFormat()
        x.Init(buf, n + offset)
        return x

    # VertexFormat
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # VertexFormat
    def Attributes(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 2
            from .VertexAttribute import VertexAttribute
            obj = VertexAttribute()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # VertexFormat
    def AttributesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # VertexFormat
    def Instanced(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return bool(self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos))
        return False

def VertexFormatStart(builder): builder.StartObject(2)
def VertexFormatAddAttributes(builder, attributes): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(attributes), 0)
def VertexFormatStartAttributesVector(builder, numElems): return builder.StartVector(2, numElems, 1)
def VertexFormatAddInstanced(builder, instanced): builder.PrependBoolSlot(1, instanced, 0)
def VertexFormatEnd(builder): return builder.EndObject()
