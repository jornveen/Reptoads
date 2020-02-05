#include "catch/catch.hpp"
#include "memory/Allocator.h"
#include <array>
#include <random>
#include "core/Assertion.h"


//TODO: Unit test this too :P
template<typename T, std::size_t MaxSize>
class BasicFixedSizeColony
{
public:
	std::array<T, MaxSize> array {};


	size_t GetEmptySlot()
	{
		for (int i = 0; i < MaxSize; ++i) {
			//This does not work at all, because a different template argument will break this 
			if (array[i].first == nullptr)
				return i;
		}

		return MaxSize + 1;
	}

	void push_back(const T& t)
	{
		auto emptyIndex = GetEmptySlot();
		array[emptyIndex] = t;
	}

	void remove(const T& t)
	{
		auto it = std::find(array.begin(), array.end(), t);
		ASSERT(it != array.end());
		*it = T{};
	}
};

template<typename T, std::size_t MaxSize>
class BasicFixedSizeStack
{
public:
	std::array<T, MaxSize> array {};
	size_t currentSize = 0;

	void push_back(const T& t)
	{
		ASSERT(currentSize < MaxSize - 1);
		array[currentSize++] = t;
	}

	void pop_back()
	{
		ASSERT(currentSize > 0);
		array[currentSize--] = T{};
	}
};

using DataCharType = unsigned char;
using FixedCharSequence = std::array<DataCharType, 20>;
const static unsigned int MaxDynamicSize = 200;
using DynamicCharSequence = BasicFixedSizeStack< DataCharType, MaxDynamicSize>;

static FixedCharSequence GetRandomFixedCharSequence(std::mt19937& mt19937)
{
	std::uniform_int_distribution<unsigned int> charDistribution{ 0, 255 };

	FixedCharSequence ret{};

	for (int i = 0; i < 20; ++i) {
		ret[i] = static_cast<DataCharType>(charDistribution(mt19937));
	}

	return ret;
}

static bool CheckMemorySection(DataCharType* ptr, const DataCharType* sequenceBegin, unsigned int count)
{
	for (int i = 0; i < count; ++i) {
		if (ptr[i] != sequenceBegin[i])
			return false;
	}

	return true;
}

static DynamicCharSequence GetRandomDynamicCharSequence(std::mt19937& mt19937)
{
	std::uniform_int_distribution<unsigned int> charDistribution{ 0, 255 };

	DynamicCharSequence ret{};

	const unsigned int size = std::uniform_int_distribution<unsigned int>{ 1, MaxDynamicSize - 1 }(mt19937);
	for (int i = 0; i < size; ++i) {
		ret.push_back(static_cast<DataCharType>(charDistribution(mt19937)));
	}

	return ret;
}



TEST_CASE("Allocate N times and then deallocate N times")
{
	std::mt19937 random_engine{ Catch::rngSeed() };

	const int AmountOfAllocationsToTest = 10'000;
	std::array< 
		std::pair<DataCharType*, FixedCharSequence>,
		AmountOfAllocationsToTest>  allocatedSections{};

	for (int i = 0; i < AmountOfAllocationsToTest; ++i) {
		auto chars = GetRandomFixedCharSequence(random_engine);
		int count = chars.size();
		int element_size = sizeof(DataCharType);

		auto* mem = reinterpret_cast<DataCharType*>(ptl::DefaultMemoryResource::get_default_allocator()->allocate(element_size * count));
		CHECK(mem != nullptr);
		memcpy(mem, chars.data(), element_size * count);
		REQUIRE(CheckMemorySection(mem, chars.data(), chars.size()));

		allocatedSections[i] = { mem, chars };
	}

	for (int i = 0; i < AmountOfAllocationsToTest; ++i) {
		DataCharType* ptr = allocatedSections[i].first;
		FixedCharSequence& sequence = allocatedSections[i].second;

		CHECK(CheckMemorySection(ptr, sequence.data(), sequence.size()));

		ptl::DefaultMemoryResource::get_default_allocator()->deallocate(ptr);
		allocatedSections[i] = {};
	}
}


const int MaxAmountOfAllocationsToTest = 100;
using MaxSizedColony = BasicFixedSizeColony < std::pair<DataCharType*, DynamicCharSequence>, MaxAmountOfAllocationsToTest>;

static size_t GetUsedSlot(MaxSizedColony& slots)
{
	for (int i = 0; i < slots.array.size(); ++i) {
		if(slots.array[i].first != nullptr) {
			return i;
		}
	}

	return MaxAmountOfAllocationsToTest + 1;
}

//Returns true if should allocate, returns false if should deallocate
static bool ShouldAllocate(std::mt19937& random_device)
{
	return std::uniform_real_distribution<float>{0.0, 1.0f}(random_device) <= 0.65f;
}
TEST_CASE("Allocate and Deallocate N times")
{
	std::mt19937 random_engine{ Catch::rngSeed() };

	MaxSizedColony allocatedSections{};

	const unsigned int AmountOfAllocationsToTest = MaxAmountOfAllocationsToTest;
	static_assert(AmountOfAllocationsToTest <= MaxAmountOfAllocationsToTest, "");

	//Randomly allocate and deallocate with a slight favor to allocation
	for (int i = 0; i < MaxAmountOfAllocationsToTest; ++i) {
		if(ShouldAllocate(random_engine)) {
			auto chars = GetRandomDynamicCharSequence(random_engine);
			int count = chars.currentSize;
			int element_size = sizeof(DataCharType);

			auto* mem = reinterpret_cast<DataCharType*>(ptl::DefaultMemoryResource::get_default_allocator()->allocate(element_size * count));
			CHECK(mem != nullptr);
			memcpy(mem, chars.array.data(), element_size * count);
			REQUIRE(CheckMemorySection(mem, chars.array.data(), chars.currentSize));

			allocatedSections.push_back({ mem, chars } );
		} else {
			//Only deallocate if there is something to deallocate
			auto usedSlot = GetUsedSlot(allocatedSections);
			if (usedSlot != MaxAmountOfAllocationsToTest + 1) {
				DataCharType* ptr = allocatedSections.array[usedSlot].first;
				DynamicCharSequence& sequence = allocatedSections.array[usedSlot].second;

				CHECK(CheckMemorySection(ptr, sequence.array.data(), sequence.currentSize));
				ptl::DefaultMemoryResource::get_default_allocator()->deallocate(ptr);
				allocatedSections.array[usedSlot] = {};
			}
		}
		
	}

	//Do final de-allocations of leftover memory
	for (int i = 0; i < AmountOfAllocationsToTest; ++i) {
		if(allocatedSections.array[i].first != nullptr) {
			DataCharType* ptr = allocatedSections.array[i].first;
			DynamicCharSequence& sequence = allocatedSections.array[i].second;

			CHECK(CheckMemorySection(ptr, sequence.array.data(), sequence.currentSize));
			ptl::DefaultMemoryResource::get_default_allocator()->deallocate(ptr);
			allocatedSections.array[i] = {};
		}
	}
}