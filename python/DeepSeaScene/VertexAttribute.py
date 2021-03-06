# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class VertexAttribute(object):
    __slots__ = ['_tab']

    # VertexAttribute
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # VertexAttribute
    def Attrib(self): return self._tab.Get(flatbuffers.number_types.Uint32Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(0))
    # VertexAttribute
    def Format(self): return self._tab.Get(flatbuffers.number_types.Uint8Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(4))
    # VertexAttribute
    def Decoration(self): return self._tab.Get(flatbuffers.number_types.Uint8Flags, self._tab.Pos + flatbuffers.number_types.UOffsetTFlags.py_type(5))

def CreateVertexAttribute(builder, attrib, format, decoration):
    builder.Prep(4, 8)
    builder.Pad(2)
    builder.PrependUint8(decoration)
    builder.PrependUint8(format)
    builder.PrependUint32(attrib)
    return builder.Offset()
