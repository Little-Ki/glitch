#pragma once

#include <cstdint>

#pragma pack(push, 0x01)
namespace ct::sdk::ue4 {
	template <typename T>
	struct TArray {
		T* raw;
		int32_t count;
		int32_t max;
	};

	struct FString : public TArray<wchar_t> {};

	struct FUObjectItem {
		class UObject* object;
		int32_t flags;
		int32_t cluster_index;
		int32_t reserved;
		int32_t serial_number;
	};

	struct TUObjectArray {
		static const constexpr int32_t num_elements_per_chunk = 0;

		int32_t max_chunks;
		int32_t num_elements;
		int32_t num_chunks;
		int reserved0;
		FUObjectItem** objects;
		char reserved1[0x10];
		int32_t max_elements;
		char reserved2[0x24];
		FUObjectItem* PreAllocatedObjects;
	};

	struct FNameEntryHeader {

		static const constexpr uint32_t probe_hash_bits = 5;
		uint16_t is_wide : 1;
		uint16_t lowercase_probe_hash : probe_hash_bits;
		uint16_t len : 10;
	};

	struct FNameEntry {
		FNameEntryHeader header;
		union {
			char name[1024];
			wchar_t w_name[1024];
		};
	};

	struct FNameEntryAllocator {
		static const constexpr int32_t stride = 0x02;
		static const constexpr int32_t max_offset = stride * (1 << 18);

		uint8_t frw_lock[0x8];
		uint8_t* blocks[8192];
		int32_t current_byte_cursor;
		int32_t current_block;
	};

	struct FNamePool {
		FNameEntryAllocator allocator;
		int32_t count;
		int32_t w_count;
	};

	struct FName {
		static FNamePool* g_names;
		int32_t comparison_index;
		int32_t number;
	};

	template <class TEnum>
	struct TEnumAsByte {
		uint8_t value;
	};

	struct FScriptInterface {
		UObject* object;
		void* interface;
	};

	template<class InterfaceType>
	struct TScriptInterface : public FScriptInterface {
	};

	struct FTextData {
		uint8_t unknown[0x28];
		wchar_t* name;
		int32_t* length;
	};

	struct FScriptDelegate {
		uint8_t unknown[0x10];
	};

	struct FScriptMulticastDelegate {
		uint8_t unknown[0x10];
	};

	template <typename K, typename V>
	struct TPair {
		K first;
		V second;
	};

	template <typename ElementType>
	struct TSetElement {
		ElementType value;
		int32_t hash_next_id;
		int32_t hash_index;
	};

	template <typename K, typename V>
	struct TMap {
		TArray<TSetElement<TPair<K, V>>> data;
	};

	struct FStringAssetReference_
	{
		uint8_t unknown[0x10];
	};

	struct FUniqueObjectGuid_
	{
		uint8_t unknown[0x10];
	};

	struct FStructBaseChain {
		FStructBaseChain** sbc_array;
		int32_t number;
	};

	struct FWeakObjectPtr {
		int32_t object_index;
		int32_t object_sn;
	};

	template <class T, class TWeakObjectPtrBase = FWeakObjectPtr>
	struct TWeakObjectPtr : public TWeakObjectPtrBase {
	};

	template <class T, class TBase>
	struct TAutoPointer : public TBase {
	};

	template <class T>
	struct TAutoWeakObjectPtr : public TAutoPointer<T, TWeakObjectPtr<T>> {
	};

	template <typename TObjectID>
	struct TPersistentObjectPtr {
		FWeakObjectPtr weak_ptr;
		int32_t tag;
		TObjectID object_id;
	};

	struct FLazyObjectPtr : public TPersistentObjectPtr<FUniqueObjectGuid_> {};

	struct FAssetPtr : public TPersistentObjectPtr<FStringAssetReference_> {};

	template <typename ObjectType>
	struct TAssetPtr : public FAssetPtr {};

	template <typename ObjectType>
	struct TLazyObjectPtr : public FLazyObjectPtr {};

	struct ObjectNames {};

}
#pragma pack(pop, 0x01)