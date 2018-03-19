# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers

class LineCommand(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsLineCommand(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = LineCommand()
        x.Init(buf, n + offset)
        return x

    # LineCommand
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # LineCommand
    def End(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = o + self._tab.Pos
            from .Vector2f import Vector2f
            obj = Vector2f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

def LineCommandStart(builder): builder.StartObject(1)
def LineCommandAddEnd(builder, end): builder.PrependStructSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(end), 0)
def LineCommandEnd(builder): return builder.EndObject()
