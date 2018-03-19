# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers

class StartPathCommand(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsStartPathCommand(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = StartPathCommand()
        x.Init(buf, n + offset)
        return x

    # StartPathCommand
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # StartPathCommand
    def Transform(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = o + self._tab.Pos
            from .Matrix33f import Matrix33f
            obj = Matrix33f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

def StartPathCommandStart(builder): builder.StartObject(1)
def StartPathCommandAddTransform(builder, transform): builder.PrependStructSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(transform), 0)
def StartPathCommandEnd(builder): return builder.EndObject()
