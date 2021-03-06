# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ModelList(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsModelList(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ModelList()
        x.Init(buf, n + offset)
        return x

    # ModelList
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # ModelList
    def InstanceData(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from DeepSeaScene.ObjectData import ObjectData
            obj = ObjectData()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # ModelList
    def InstanceDataLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # ModelList
    def InstanceDataIsNone(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        return o == 0

    # ModelList
    def SortType(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # ModelList
    def DynamicRenderStates(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = self._tab.Indirect(o + self._tab.Pos)
            from DeepSeaScene.DynamicRenderStates import DynamicRenderStates
            obj = DynamicRenderStates()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # ModelList
    def CullName(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def ModelListStart(builder): builder.StartObject(4)
def ModelListAddInstanceData(builder, instanceData): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(instanceData), 0)
def ModelListStartInstanceDataVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def ModelListAddSortType(builder, sortType): builder.PrependUint8Slot(1, sortType, 0)
def ModelListAddDynamicRenderStates(builder, dynamicRenderStates): builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(dynamicRenderStates), 0)
def ModelListAddCullName(builder, cullName): builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(cullName), 0)
def ModelListEnd(builder): return builder.EndObject()
