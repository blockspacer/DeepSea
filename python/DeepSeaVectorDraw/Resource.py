# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers

class Resource(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsResource(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Resource()
        x.Init(buf, n + offset)
        return x

    # Resource
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Resource
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return bytes()

    # Resource
    def Path(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return bytes()

def ResourceStart(builder): builder.StartObject(2)
def ResourceAddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def ResourceAddPath(builder, path): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(path), 0)
def ResourceEnd(builder): return builder.EndObject()