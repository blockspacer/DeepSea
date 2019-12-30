# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers

class SceneNode(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsSceneNode(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = SceneNode()
        x.Init(buf, n + offset)
        return x

    # SceneNode
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # SceneNode
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

    # SceneNode
    def Node(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Indirect(o + self._tab.Pos)
            from .ObjectData import ObjectData
            obj = ObjectData()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

def SceneNodeStart(builder): builder.StartObject(2)
def SceneNodeAddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def SceneNodeAddNode(builder, node): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(node), 0)
def SceneNodeEnd(builder): return builder.EndObject()
