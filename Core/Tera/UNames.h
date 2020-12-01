#pragma once

#define DECL_NAME(name) extern const char* NAME_##name

DECL_NAME(None);

DECL_NAME(ByteProperty);
DECL_NAME(IntProperty);
DECL_NAME(BoolProperty);
DECL_NAME(FloatProperty);
DECL_NAME(ObjectProperty);
DECL_NAME(NameProperty);
DECL_NAME(DelegateProperty);
DECL_NAME(ClassProperty);
DECL_NAME(ArrayProperty);
DECL_NAME(StructProperty);
DECL_NAME(VectorProperty);
DECL_NAME(RotatorProperty);
DECL_NAME(StrProperty);
DECL_NAME(MapProperty);
DECL_NAME(InterfaceProperty);
DECL_NAME(ComponentProperty);

DECL_NAME(Byte);
DECL_NAME(Int);
DECL_NAME(Bool);
DECL_NAME(Float);
DECL_NAME(Name);
DECL_NAME(Enum);
DECL_NAME(Const);
DECL_NAME(String);
DECL_NAME(Struct);
DECL_NAME(ScriptStruct);
DECL_NAME(Vector);
DECL_NAME(Rotator);
DECL_NAME(SHVector);
DECL_NAME(Color);
DECL_NAME(Plane);
DECL_NAME(Button);
DECL_NAME(Matrix);
DECL_NAME(LinearColor);
DECL_NAME(QWord);
DECL_NAME(Pointer);
DECL_NAME(Double);
DECL_NAME(Quat);
DECL_NAME(Vector4);

DECL_NAME(Field);
DECL_NAME(State);
DECL_NAME(Function);
DECL_NAME(Package);
DECL_NAME(Object);
DECL_NAME(TextBuffer);
DECL_NAME(Linker);
DECL_NAME(LinkerLoad);
DECL_NAME(LinkerSave);
DECL_NAME(Subsystem);
DECL_NAME(Factory);
DECL_NAME(TextBufferFactory);
DECL_NAME(Exporter);
DECL_NAME(StackNode);
DECL_NAME(Property);
DECL_NAME(Camera);
DECL_NAME(PlayerInput);
DECL_NAME(Actor);
DECL_NAME(ActorFactory);
DECL_NAME(ObjectRedirector);
DECL_NAME(ObjectArchetype);
DECL_NAME(Component);
DECL_NAME(MetaData);
DECL_NAME(StaticMesh);
DECL_NAME(Level);
DECL_NAME(PersistentCookerData);
DECL_NAME(Class);

DECL_NAME(WorldTextures);