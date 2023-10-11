// Copyright 2012 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_HEAP_OBJECTS_VISITING_H_
#define V8_HEAP_OBJECTS_VISITING_H_

#include "src/base/logging.h"
#include "src/objects/bytecode-array.h"
#include "src/objects/contexts.h"
#include "src/objects/fixed-array.h"
#include "src/objects/js-weak-refs.h"
#include "src/objects/map.h"
#include "src/objects/object-list-macros.h"
#include "src/objects/objects.h"
#include "src/objects/shared-function-info.h"
#include "src/objects/string.h"
#include "src/objects/visitors.h"

namespace v8 {
namespace internal {

#define TYPED_VISITOR_ID_LIST(V)        \
  V(AccessorInfo)                       \
  V(AllocationSite)                     \
  V(ArrayList)                          \
  V(BigInt)                             \
  V(ByteArray)                          \
  V(BytecodeArray)                      \
  V(ExternalPointerArray)               \
  V(CallHandlerInfo)                    \
  V(Cell)                               \
  V(InstructionStream)                  \
  V(ClosureFeedbackCellArray)           \
  V(Code)                               \
  V(CoverageInfo)                       \
  V(DataHandler)                        \
  V(EmbedderDataArray)                  \
  V(EphemeronHashTable)                 \
  V(ExternalString)                     \
  V(FeedbackCell)                       \
  V(FeedbackMetadata)                   \
  V(FixedArray)                         \
  V(FixedDoubleArray)                   \
  V(JSArrayBuffer)                      \
  V(JSDataViewOrRabGsabDataView)        \
  V(JSExternalObject)                   \
  V(JSFinalizationRegistry)             \
  V(JSFunction)                         \
  V(JSObject)                           \
  V(JSSynchronizationPrimitive)         \
  V(JSTypedArray)                       \
  V(WeakCell)                           \
  V(JSWeakCollection)                   \
  V(JSWeakRef)                          \
  V(Map)                                \
  V(NativeContext)                      \
  V(ObjectBoilerplateDescription)       \
  V(Oddball)                            \
  V(Hole)                               \
  V(PreparseData)                       \
  V(PromiseOnStack)                     \
  V(PropertyArray)                      \
  V(PropertyCell)                       \
  V(PrototypeInfo)                      \
  V(RegExpMatchInfo)                    \
  V(SharedFunctionInfo)                 \
  V(SloppyArgumentsElements)            \
  V(SmallOrderedHashMap)                \
  V(SmallOrderedHashSet)                \
  V(SmallOrderedNameDictionary)         \
  V(SourceTextModule)                   \
  V(SwissNameDictionary)                \
  V(Symbol)                             \
  V(SyntheticModule)                    \
  V(TransitionArray)                    \
  IF_WASM(V, WasmApiFunctionRef)        \
  IF_WASM(V, WasmArray)                 \
  IF_WASM(V, WasmCapiFunctionData)      \
  IF_WASM(V, WasmExportedFunctionData)  \
  IF_WASM(V, WasmFunctionData)          \
  IF_WASM(V, WasmIndirectFunctionTable) \
  IF_WASM(V, WasmInstanceObject)        \
  IF_WASM(V, WasmInternalFunction)      \
  IF_WASM(V, WasmJSFunctionData)        \
  IF_WASM(V, WasmStruct)                \
  IF_WASM(V, WasmSuspenderObject)       \
  IF_WASM(V, WasmResumeData)            \
  IF_WASM(V, WasmTypeInfo)              \
  IF_WASM(V, WasmContinuationObject)    \
  IF_WASM(V, WasmNull)

#define FORWARD_DECLARE(TypeName) class TypeName;
TYPED_VISITOR_ID_LIST(FORWARD_DECLARE)
TORQUE_VISITOR_ID_LIST(FORWARD_DECLARE)
#undef FORWARD_DECLARE

// The base class for visitors that need to dispatch on object type. The default
// behavior of all visit functions is to iterate body of the given object using
// the BodyDescriptor of the object.
//
// The visit functions return the size of the object cast to ResultType.
//
// This class is intended to be used in the following way:
//
//   class SomeVisitor : public HeapVisitor<ResultType, SomeVisitor> {
//     ...
//   }
template <typename ResultType, typename ConcreteVisitor>
class HeapVisitor : public ObjectVisitorWithCageBases {
 public:
  inline HeapVisitor(PtrComprCageBase cage_base,
                     PtrComprCageBase code_cage_base);
  inline explicit HeapVisitor(Isolate* isolate);
  inline explicit HeapVisitor(Heap* heap);

  V8_INLINE ResultType Visit(Tagged<HeapObject> object);
  V8_INLINE ResultType Visit(Tagged<Map> map, Tagged<HeapObject> object);

 protected:
  // If this predicate returns false the default implementations of Visit*
  // functions bail out from visiting the map pointer.
  V8_INLINE static constexpr bool ShouldVisitMapPointer() { return true; }
  // If this predicate returns false the default implementations of Visit*
  // functions bail out from visiting known read-only maps.
  V8_INLINE static constexpr bool ShouldVisitReadOnlyMapPointer() {
    return true;
  }

  // Only visits the Map pointer if `ShouldVisitMapPointer()` returns true.
  template <VisitorId visitor_id>
  V8_INLINE void VisitMapPointerIfNeeded(Tagged<HeapObject> host);

  ConcreteVisitor* concrete_visitor() {
    return static_cast<ConcreteVisitor*>(this);
  }

  const ConcreteVisitor* concrete_visitor() const {
    return static_cast<const ConcreteVisitor*>(this);
  }

#define VISIT(TypeName)                                 \
  V8_INLINE ResultType Visit##TypeName(Tagged<Map> map, \
                                       Tagged<TypeName> object);
  TYPED_VISITOR_ID_LIST(VISIT)
  TORQUE_VISITOR_ID_LIST(VISIT)
#undef VISIT
  V8_INLINE ResultType VisitShortcutCandidate(Tagged<Map> map,
                                              Tagged<ConsString> object);
  V8_INLINE ResultType VisitDataObject(Tagged<Map> map,
                                       Tagged<HeapObject> object);
  V8_INLINE ResultType VisitJSObjectFast(Tagged<Map> map,
                                         Tagged<JSObject> object);
  V8_INLINE ResultType VisitJSApiObject(Tagged<Map> map,
                                        Tagged<JSObject> object);
  V8_INLINE ResultType VisitStruct(Tagged<Map> map, Tagged<HeapObject> object);
  V8_INLINE ResultType VisitFreeSpace(Tagged<Map> map,
                                      Tagged<FreeSpace> object);

  template <typename T, typename TBodyDescriptor = typename T::BodyDescriptor>
  V8_INLINE ResultType VisitJSObjectSubclass(Tagged<Map> map, Tagged<T> object);

  template <typename T>
  static V8_INLINE Tagged<T> Cast(Tagged<HeapObject> object);
};

// These strings can be sources of safe string transitions. Transitions are safe
// if they don't result in invalidated slots. It's safe to read the length field
// on such strings as that's common for all.
//
// No special visitors are generated for such strings.
// V(VisitorId, TypeName)
#define SAFE_STRING_TRANSITION_SOURCES(V) \
  V(SeqOneByteString, SeqOneByteString)   \
  V(SeqTwoByteString, SeqTwoByteString)

// These strings can be sources of unsafe string transitions.
// V(VisitorId, TypeName)
#define UNSAFE_STRING_TRANSITION_SOURCES(V) \
  V(ExternalString, ExternalString)         \
  V(ConsString, ConsString)                 \
  V(SlicedString, SlicedString)

// V(VisitorId, TypeName)
#define UNSAFE_STRING_TRANSITION_TARGETS(V) \
  UNSAFE_STRING_TRANSITION_SOURCES(V)       \
  V(ShortcutCandidate, ConsString)          \
  V(ThinString, ThinString)

// A HeapVisitor that allows for concurrently tracing through objects. Tracing
// through objects with unsafe shape changes is guarded by
// `EnableConcurrentVisitation()` which defaults to off.
template <typename ResultType, typename ConcreteVisitor>
class ConcurrentHeapVisitor : public HeapVisitor<ResultType, ConcreteVisitor> {
 public:
  V8_INLINE explicit ConcurrentHeapVisitor(Isolate* isolate);

  V8_INLINE static constexpr bool EnableConcurrentVisitation() { return false; }

 protected:
#define VISIT_AS_LOCKED_STRING(VisitorId, TypeName)     \
  V8_INLINE ResultType Visit##TypeName(Tagged<Map> map, \
                                       Tagged<TypeName> object);

  UNSAFE_STRING_TRANSITION_SOURCES(VISIT_AS_LOCKED_STRING)
#undef VISIT_AS_LOCKED_STRING

  template <typename T>
  static V8_INLINE Tagged<T> Cast(Tagged<HeapObject> object);

 private:
  template <typename T>
  V8_INLINE ResultType VisitStringLocked(Tagged<T> object);

  friend class HeapVisitor<ResultType, ConcreteVisitor>;
};

template <typename ConcreteVisitor>
class NewSpaceVisitor : public ConcurrentHeapVisitor<int, ConcreteVisitor> {
 public:
  V8_INLINE explicit NewSpaceVisitor(Isolate* isolate);

  // Special cases: Unreachable visitors for objects that are never found in the
  // young generation.
  void VisitInstructionStreamPointer(Tagged<Code>,
                                     InstructionStreamSlot) final {
    UNREACHABLE();
  }
  void VisitCodeTarget(Tagged<InstructionStream> host, RelocInfo*) final {
    UNREACHABLE();
  }
  void VisitEmbeddedPointer(Tagged<InstructionStream> host, RelocInfo*) final {
    UNREACHABLE();
  }
  void VisitMapPointer(Tagged<HeapObject>) override { UNREACHABLE(); }

 protected:
  V8_INLINE static constexpr bool ShouldVisitMapPointer() { return false; }

  // Special cases: Unreachable visitors for objects that are never found in the
  // young generation.
  int VisitNativeContext(Tagged<Map>, Tagged<NativeContext>) { UNREACHABLE(); }
  int VisitBytecodeArray(Tagged<Map>, Tagged<BytecodeArray>) { UNREACHABLE(); }
  int VisitSharedFunctionInfo(Tagged<Map> map, Tagged<SharedFunctionInfo>) {
    UNREACHABLE();
  }
  int VisitWeakCell(Tagged<Map>, Tagged<WeakCell>) { UNREACHABLE(); }

  friend class HeapVisitor<int, ConcreteVisitor>;
};

class WeakObjectRetainer;

// A weak list is single linked list where each element has a weak pointer to
// the next element. Given the head of the list, this function removes dead
// elements from the list and if requested records slots for next-element
// pointers. The template parameter T is a WeakListVisitor that defines how to
// access the next-element pointers.
template <class T>
Tagged<Object> VisitWeakList(Heap* heap, Tagged<Object> list,
                             WeakObjectRetainer* retainer);
}  // namespace internal
}  // namespace v8

#endif  // V8_HEAP_OBJECTS_VISITING_H_
