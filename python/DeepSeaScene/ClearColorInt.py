# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers

class ClearColorInt(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsClearColorInt(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ClearColorInt()
        x.Init(buf, n + offset)
        return x

    # ClearColorInt
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ClearColorInt
    def Red(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int32Flags, o + self._tab.Pos)
        return 0

    # ClearColorInt
    def Green(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int32Flags, o + self._tab.Pos)
        return 0

    # ClearColorInt
    def Blue(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int32Flags, o + self._tab.Pos)
        return 0

    # ClearColorInt
    def Alpha(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int32Flags, o + self._tab.Pos)
        return 0

def ClearColorIntStart(builder): builder.StartObject(4)
def ClearColorIntAddRed(builder, red): builder.PrependInt32Slot(0, red, 0)
def ClearColorIntAddGreen(builder, green): builder.PrependInt32Slot(1, green, 0)
def ClearColorIntAddBlue(builder, blue): builder.PrependInt32Slot(2, blue, 0)
def ClearColorIntAddAlpha(builder, alpha): builder.PrependInt32Slot(3, alpha, 0)
def ClearColorIntEnd(builder): return builder.EndObject()
