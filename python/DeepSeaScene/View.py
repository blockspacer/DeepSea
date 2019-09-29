# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaScene

import flatbuffers

class View(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsView(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = View()
        x.Init(buf, n + offset)
        return x

    # View
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # View
    def Surfaces(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .Surface import Surface
            obj = Surface()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # View
    def SurfacesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # View
    def Framebuffers(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .Framebuffer import Framebuffer
            obj = Framebuffer()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # View
    def FramebuffersLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

def ViewStart(builder): builder.StartObject(2)
def ViewAddSurfaces(builder, surfaces): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(surfaces), 0)
def ViewStartSurfacesVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def ViewAddFramebuffers(builder, framebuffers): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(framebuffers), 0)
def ViewStartFramebuffersVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def ViewEnd(builder): return builder.EndObject()
