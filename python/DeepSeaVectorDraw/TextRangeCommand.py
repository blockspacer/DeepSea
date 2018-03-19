# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers

class TextRangeCommand(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsTextRangeCommand(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = TextRangeCommand()
        x.Init(buf, n + offset)
        return x

    # TextRangeCommand
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # TextRangeCommand
    def Start(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # TextRangeCommand
    def Count(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint32Flags, o + self._tab.Pos)
        return 0

    # TextRangeCommand
    def PositionType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # TextRangeCommand
    def Position(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            x = o + self._tab.Pos
            from .Vector2f import Vector2f
            obj = Vector2f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # TextRangeCommand
    def FillMaterial(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return bytes()

    # TextRangeCommand
    def OutlineMaterial(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return bytes()

    # TextRangeCommand
    def FillOpacity(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(16))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # TextRangeCommand
    def OutlineOpacity(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(18))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # TextRangeCommand
    def Size(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(20))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # TextRangeCommand
    def Embolden(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(22))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # TextRangeCommand
    def Slant(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(24))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

    # TextRangeCommand
    def OutlineWidth(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(26))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Float32Flags, o + self._tab.Pos)
        return 0.0

def TextRangeCommandStart(builder): builder.StartObject(12)
def TextRangeCommandAddStart(builder, start): builder.PrependUint32Slot(0, start, 0)
def TextRangeCommandAddCount(builder, count): builder.PrependUint32Slot(1, count, 0)
def TextRangeCommandAddPositionType(builder, positionType): builder.PrependUint8Slot(2, positionType, 0)
def TextRangeCommandAddPosition(builder, position): builder.PrependStructSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(position), 0)
def TextRangeCommandAddFillMaterial(builder, fillMaterial): builder.PrependUOffsetTRelativeSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(fillMaterial), 0)
def TextRangeCommandAddOutlineMaterial(builder, outlineMaterial): builder.PrependUOffsetTRelativeSlot(5, flatbuffers.number_types.UOffsetTFlags.py_type(outlineMaterial), 0)
def TextRangeCommandAddFillOpacity(builder, fillOpacity): builder.PrependFloat32Slot(6, fillOpacity, 0.0)
def TextRangeCommandAddOutlineOpacity(builder, outlineOpacity): builder.PrependFloat32Slot(7, outlineOpacity, 0.0)
def TextRangeCommandAddSize(builder, size): builder.PrependFloat32Slot(8, size, 0.0)
def TextRangeCommandAddEmbolden(builder, embolden): builder.PrependFloat32Slot(9, embolden, 0.0)
def TextRangeCommandAddSlant(builder, slant): builder.PrependFloat32Slot(10, slant, 0.0)
def TextRangeCommandAddOutlineWidth(builder, outlineWidth): builder.PrependFloat32Slot(11, outlineWidth, 0.0)
def TextRangeCommandEnd(builder): return builder.EndObject()