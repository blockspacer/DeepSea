# automatically generated by the FlatBuffers compiler, do not modify

# namespace: DeepSeaVectorDraw

import flatbuffers

class Font(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAsFont(cls, buf, offset):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Font()
        x.Init(buf, n + offset)
        return x

    # Font
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Font
    def Name(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return bytes()

    # Font
    def FaceGroup(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return bytes()

    # Font
    def Faces(self, j):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            a = self._tab.Vector(o)
            return self._tab.String(a + flatbuffers.number_types.UOffsetTFlags.py_type(j * 4))
        return ""

    # Font
    def FacesLength(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(8))
        if o != 0:
            return self._tab.VectorLen(o)
        return 0

    # Font
    def Quality(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(10))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

    # Font
    def CacheSize(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(12))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Uint8Flags, o + self._tab.Pos)
        return 0

def FontStart(builder): builder.StartObject(5)
def FontAddName(builder, name): builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(name), 0)
def FontAddFaceGroup(builder, faceGroup): builder.PrependUOffsetTRelativeSlot(1, flatbuffers.number_types.UOffsetTFlags.py_type(faceGroup), 0)
def FontAddFaces(builder, faces): builder.PrependUOffsetTRelativeSlot(2, flatbuffers.number_types.UOffsetTFlags.py_type(faces), 0)
def FontStartFacesVector(builder, numElems): return builder.StartVector(4, numElems, 4)
def FontAddQuality(builder, quality): builder.PrependUint8Slot(3, quality, 0)
def FontAddCacheSize(builder, cacheSize): builder.PrependUint8Slot(4, cacheSize, 0)
def FontEnd(builder): return builder.EndObject()
