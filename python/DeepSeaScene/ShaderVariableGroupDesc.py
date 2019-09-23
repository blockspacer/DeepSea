# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers

class ShaderVariableGroupDesc(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsShaderVariableGroupDesc(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ShaderVariableGroupDesc()
        x.Init(buf, n + offset)
        return x

    # ShaderVariableGroupDesc
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ShaderVariableGroupDesc
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # ShaderVariableGroupDesc
    def Elements(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .VariableElement import VariableElement
            obj = VariableElement()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # ShaderVariableGroupDesc
    def ElementsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

def ShaderVariableGroupDescStart(builder): builder.StartObject(2)
def ShaderVariableGroupDescAddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def ShaderVariableGroupDescAddElements(builder, elements): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(elements), 0)
def ShaderVariableGroupDescStartElementsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def ShaderVariableGroupDescEnd(builder): return builder.EndObject()
