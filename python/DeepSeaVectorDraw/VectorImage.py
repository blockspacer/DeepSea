# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers

class VectorImage(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsVectorImage(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = VectorImage()
        x.Init(buf, n + offset)
        return x

    # VectorImage
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # VectorImage
    def ColorMaterials(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .ColorMaterial import ColorMaterial
            obj = ColorMaterial()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # VectorImage
    def ColorMaterialsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # VectorImage
    def LinearGradients(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .LinearGradient import LinearGradient
            obj = LinearGradient()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # VectorImage
    def LinearGradientsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # VectorImage
    def RadialGradients(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .RadialGradient import RadialGradient
            obj = RadialGradient()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # VectorImage
    def RadialGradientsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # VectorImage
    def Commands(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            x = self._tab.Vector(o)
            x += flatbuffers.number_types.UOffsetTFlags.py_type(j) * 4
            x = self._tab.Indirect(x)
            from .VectorCommand import VectorCommand
            obj = VectorCommand()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # VectorImage
    def CommandsLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # VectorImage
    def Size(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            x = o + self._tab.Pos
            from .Vector2f import Vector2f
            obj = Vector2f()
            obj.Init(self._tab.Bytes, x)
            return obj
        return None

    # VectorImage
    def SRGB(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(14))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.BoolFlags, o + self._tab.Pos)
        return 0

def VectorImageStart(builder): builder.StartObject(6)
def VectorImageAddColorMaterials(builder, colorMaterials): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(colorMaterials), 0)
def VectorImageStartColorMaterialsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def VectorImageAddLinearGradients(builder, linearGradients): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(linearGradients), 0)
def VectorImageStartLinearGradientsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def VectorImageAddRadialGradients(builder, radialGradients): builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(radialGradients), 0)
def VectorImageStartRadialGradientsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def VectorImageAddCommands(builder, commands): builder.PrependUOffsetTRelativeSlot(3, flatbuffers.number_types.UOffsetTFlags.py_type(commands), 0)
def VectorImageStartCommandsVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def VectorImageAddSize(builder, size): builder.PrependStructSlot(4, flatbuffers.number_types.UOffsetTFlags.py_type(size), 0)
def VectorImageAddSRGB(builder, sRGB): builder.PrependBoolSlot(5, sRGB, 0)
def VectorImageEnd(builder): return builder.EndObject()
