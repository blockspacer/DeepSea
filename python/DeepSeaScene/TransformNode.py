# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers

class TransformNode(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsTransformNode(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = TransformNode()
        x.Init(buf, n + offset)
        return x

    # TransformNode
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # TransformNode
    def Transform(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = o + self._tab.Pos
            from .Matrix44f import Matrix44f
            obj = Matrix44f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # TransformNode
    def Children(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .ObjectData import ObjectData
            obj = ObjectData()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # TransformNode
    def ChildrenLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

def TransformNodeStart(builder): builder.StartObject(2)
def TransformNodeAddTransform(builder, transform): builder.PrependStructSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(transform), 0)
def TransformNodeAddChildren(builder, children): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(children), 0)
def TransformNodeStartChildrenVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def TransformNodeEnd(builder): return builder.EndObject()
