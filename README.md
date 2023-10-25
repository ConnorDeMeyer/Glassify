# Glassify
Glassify is a simple customizable Header-Only Reflection System for C++.
It allows the user to easily reflect upon types wherever they are inside of the codebase. The reflection data is generated before the main function is executed. The developer can choose which data gets stored inside of the reflection system by adding code to the config files.

## Requisites
C++ 20

## Installation
Simply include the `include` folder into your project.
`#include "glassify.h"` should be used at the top of any file that wishes to use the reflection system.

### Example directory
The example directory should not be included into an existing project and only exists for testing and providing basic examples.

## Customizing Glassify
some features of glassify are able to be customized to fit the needs of the developer:

The developer is able to choose which reflection data is stored inside the `glas::TypeInfo` struct by modyfing the contents of the `glas_config.h` file.

Inside of the config file the developer may add any variable that he might need for his program inside of the TypeInfo struct.

The newly added variable should then be filled inside of the `TypeInfo::Create()` function below the struct definition.

There are addons that can be toggled on or off by commenting out the `#define` for each addon

### glas_config.h
the `glas_config.h` file contains information about the information that will be stored for each type.
the information that is stored by default is:
 - Name of the Type
 - Size of the type
 - Alignment of the type
 - v-table pointer in case the type is polymorphic (experimental feature)
 - Vector of Member variable info owned by the type
 - Vector of Methods owned by the type
 - Base classes of the type
 - Child classes of the type

It also contains 2 optional addons to the system:
###### Storage
 - Function pointer to construct the type
 - Function pointer to copy construct the type
 - Function pointer to move construct the type
 - Function pointer to destruct the type
 - Function pointer to swap the type
###### Serialization
- Function pointer to serialize the type to json
- Function pointer to serialize the type to binary
- Function pointer to deserialize the type from json
- Function pointer to deserialize the type from binary
###### Custom
 This section is reserved for the developer to add any necessary information

### glas_properties.h
the `glas_properties.h` file contains customizable properties that can be placed on member variables and functions.

## Usage

### Reflecting Types
Types can be added to the reflection system by using the `GLAS_TYPE()` macro:
```cpp
struct Vector
{
	float X{}, Y{}, Z{};
};

GLAS_TYPE(Vector);
```

#### Type Identifier
A `glas::TypeId` is a class containing a hash that is unique for each Type. The hash is the same between program invokations as long as the Name of the type does not change.
The `TypeId` has a function called `GetInfo()` which will return the type info associated with the TypeId (if the type is registered to the reflection system).

You can get a `TypeId` by using the function `Glas::TypeId::Create<TYPE>()`, where TYPE is the name of the type you want to create a `TypeId` for.

You may access all registered `TypeId` and their perspective `TypeInfo` using the function:
```cpp
glas::GetAllTypeInfo()
```

#### Type Info
`glas::TypeInfo` is a struct containing the type information that can normally only be accessed at compile time. the information in this struct can be customized inside of the `glas_config.h` file.


### Reflecting Member Variables
Member variables can be added to the reflection system by using the `GLAS_MEMBER(CLASS, MEMBER)`. `CLASS` is the owning class type and `MEMBER` is the name of the member variable.
```cpp
struct Vector
{
	float X{}, Y{}, Z{};
};

GLAS_MEMBER(Vector, X);
GLAS_MEMBER(Vector, Y);
GLAS_MEMBER(Vector, Z);
```
#### Variable Identifier
Variables can be identified using a `glas::VariableId`. This type holds a `glas::TypeId` and keeps track of the modifying factors like if it is _const_, _volatile_, _reference_, _r value reference_, _pointers_, _Array size_. This information can be freely queried and modified depending on the use of the type. It also contains a `ToString` method that allows for better representation of the variable type.

### Reflecting Functions
Functions and member functions can be reflected using Glassify. using the macros `GLAS_FUNCTION(Function)` and `GLAS_MEMBER_FUNCTION(Class, MemberFunction)` for functions and member functions.

Each reflected _function/member function_ will be assigned a unique _ID_ that stays the same between sessions as long as the function signature does not change (no name change and no parameter change). The identifier gets constructed from the has of the name, parameters and, in case of a member function, the class it belongs to.

Yo are able to get the ID of a _function/member function_ using the macros `GLAS_FUNCTION_ID(Function)` and `GLAS_MEMBER_FUNCTION_ID(Class, Function)`.

```cpp
int AnyFunction(double AnyParameters);

class AnyClass
{
public:
	void AnyMemberFunction();
}

GLASS_FUNCTION(AnyFunction);
GLASS_MEMBER_FUNCTION(AnyClass, AnyMemberFunction);
```

#### Function Information
The information that gets stored for any function is the following:
 - Function Address
 - Return Type
 - Name
 - Hash of Return Type and Parameter Types
 - Parameter Types
 - Properties

#### Function Casting
It is possible to try and dynamically cast any address to a specific function type using the `Cast` member function. If the cast is successful it will return a valid pointer, if unsuccessful it will return `nullptr`.

#### Function Calling
Using the `TypeTuple` class from the [Storage](#storage) addon it is possible to call a function at runtime using data that is generated at runtime given that the parameters share the same types as the variables inside the `TypeTuple`.

### Reflecting Parent-Child relation ships
The relation ship between a parent and child class can also be registered into the reflection system by using the Macro `GLAS_CHILD`.

```cpp
class BaseClass
{};

class ChildClass : public BaseClass
{};

GLAS_CHILD(BaseClass, ChildClass);
```

### Dependancies

Some Types may need to rely on other types in order to maintain functionality. For examples: If we want to serialize an `std::vector<int>` than we need to register both the `std::vector<int>` and `int` to the serialization to work correctly. In this case the `std::vector<int>` depends on `int` for the type to work correctly.

Using the type `Glas::AddDependancy` we can define this relationship and automatically add any type which is depended on by another class, example:
```cpp
namespace glas
{
	template <typename T>
	struct AddDependency<std::vector<T>>
	{
		inline static AutoRegisterTypeOnce<T> RegisterValue{};
	};

	template <>
	struct AddDependency<Transform>
	{
		inline static AutoRegisterTypeOnce<ComponentBase> RegisterComponent{};
	};
}
```
Please look inside of `glas_dependencies.h` for more examples. These `AddDependency` structs can be created anywhere inside of the project.

# Addons
Addons are useful features that come with Glassify that can be completely removed from the project if the developer has no need for them

## Serialization

This features allows the user to serialize a class to either JSon or Binary. The `#define GLAS_SERIALIZATION` macro must be defined inside of the `glas_config.h` file. In order to serialize member variables, the `GLAS_MEMBER` macro must be used to register the member variables into the reflection system.

### Usage

```cpp
glas::Serialization::SerializeType(std::ostream& stream, const void* data, TypeId type)
glas::Serialization::SerializeType(std::ostream& stream, const TYPE& type)
```
These function allow the user to serialize a type to a stream in JSon format.

```cpp
glas::Serialization::SerializeTypeBinary(std::ostream& stream, const void* data, TypeId type)
glas::Serialization::SerializeTypeBinary(std::ostream& stream, const TYPE& type)
```
These function allow the user to serialize a type to a stream in Binary format.

```cpp
glas::Serialization::DeserializeType(std::istream& stream, void* data, TypeId type)
glas::Serialization::DeserializeType(std::ostream& stream, const TYPE& type)
```
These function allow the user to deserialize a type from a stream that is in JSon format. It will fill the values gotten from the stream into the instance of the type.

```cpp
glas::Serialization::DeserializeTypeBinary(std::istream& stream, void* data, TypeId type)
glas::Serialization::DeserializeTypeBinary(std::ostream& stream, const TYPE& type)
```
These function allow the user to deserialize a type from a stream that is in Binary format. It will fill the values gotten from the stream into the instance of the type.

### Customizing

The developer can implement their own type Serialization functions to be used inside of the Serialization system. There are 2 ways to implement serialization for custom types:
1. Inside of the custom type that wants to be serialized there must exist specific functions with the right naming convention and parameters that the Serialization system can call. This method works great for higher level code (example: GameComponents)
2. Declare and define the custom serialization functions inside the `glas_serialization_config.h` and the `glas_serialization.h` file. This method must be used when using types that cannot be modified like classes from low level libraries and different frameworks (example: std containers).

Please look at the `glas_serialization.h` file to see examples.

#### Method 1: Serialization defined outside the Serialization System headers.
To implement serialization for custom types the user must add the following 2 public methods inside of the class for JSon serialization:
```cpp
void GlasSerialize(std::ostream& oStream) const;
void GlasDeserialize(std::istream& iStream);
```
and the following 2 for binary serialization:
```cpp
void GlasSerializeBinary(std::ostream& oStream) const;
void GlasDeserializeBinary(std::istream& iStream);
```

You can query whether the functions are correctly implemented by using the following static asserts:
```cpp
static_assert(CustomSerializer<TYPE>);
static_assert(CustomSerializerBinary<TYPE>);
static_assert(CustomDeserializer<TYPE>);
static_assert(CustomDeserializerBinary<TYPE>);
```

#### Method 2: Serialization defined inside the Serialization System headers
For this method you must modify the `glas_serialization.h` and `glas_serialization_config.h` files. The serialization functions must be declared before the Serialization System implementation in the `glas_serialization.h` file. The serialization functions are declared inside of the `glas_serialization_config.h` file and they are implemented inside of the `glas_serialization.h` file.
To add JSon serialization for a custom type, the developer must declare and define the following functions:
```cpp
inline void Serialize(std::ostream& stream, const TYPE& value);
inline void Deserialize(std::istream& stream, TYPE& value);
```
To add binary Serialization for a custom type, the following functions must be declared and defined:
```cpp
inline void SerializeBinary(std::ostream& stream, const TYPE& value);
inline void DeserializeBinary(std::istream& stream, TYPE& value);
```

## Storage

This feature allows the user to store and initialize instances of types at runtime using only the `glas::TypeId`.

### Settings


### Type Storage
Similarly to an std::unique_ptr, the `TypeStorage` class allows for the instanciation of any class that has been added to the reflection system. The class will instanciate a given type on the Heap and is responsible for safely destroying them.

### Shared Type Storage

Similarly to an std::shared_ptr, the `SharedTypeStorage` class allows instanciation of any class that has been added to the reflection system and can be easily copied and shared without the type instance going out of scope.

### Weak Type Storage

Similarly to an std::weak_ptr, the `WeakTypeStorage` class needs to `SharedTypeStorage` to initialize and will hold a reference to that instance until there are no more shared type storages left.

### Type Vector

Similarly to an std::vector, the `TypeVector` stores a contigious array of instances of a type. this vector can be freely added to, removed from and queried. There are also iterator that can be used to iterator over the elements. The whole vector is typles and will store only data. The user is responsible for interpreting the data.

### Type Tuple
The `TypeTuple` class works similarly to a `std::tuple` class but typeless. It stores multiple instances of types and keeps track of the types inside using an array of `VariableId`s. It can be used to call functions and can be serialized too.

