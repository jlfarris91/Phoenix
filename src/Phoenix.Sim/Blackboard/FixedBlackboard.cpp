#include "Phoenix.Sim/Blackboard/FixedBlackboard.h"

using namespace Phoenix;
using namespace Phoenix::Blackboard;

struct CompareKeyHi
{
    bool operator()(blackboard_key_t keyA, blackboard_key_t keyB) const noexcept
    {
        return BlackboardKey::GetKeyHi(keyA) == BlackboardKey::GetKeyHi(keyB);
    }
};

BlackboardItem::BlackboardItem(blackboard_key_t key, blackboard_value_t value)
    : Key(key)
    , Value(value)
{
}

bool BlackboardItem::operator==(const BlackboardItem& other) const
{
    return Key == other.Key && Value == other.Value;
}

bool BlackboardItem::IsValid() const
{
    return BlackboardKey::GetKeyType(Key) != (uint8)EBlackboardValueTypes::Invalid;
}

void BlackboardItem::Invalidate()
{
    Key = BlackboardKey::Create(Key, (uint8)EBlackboardValueTypes::Invalid);
}

blackboard_key_t BlackboardItem::GetItemKey::operator()(const BlackboardItem& item) const
{
    return item.Key;
}

BlackboardQuery::BlackboardQuery(blackboard_key_t key): Filter(key)
{
}

BlackboardQuery::BlackboardQuery(blackboard_key_t key, blackboard_type_t type): Filter(BlackboardKey::Create(key, type))
{
}

BlackboardQuery::BlackboardQuery(uint32 keyLo, uint32 keyHi, blackboard_type_t type): Filter(BlackboardKey::Create(keyLo, keyHi, type))
{
}

BlackboardQuery BlackboardQuery::WithType(blackboard_type_t type) const
{
    return BlackboardQuery(Filter, type);
}

bool BlackboardQuery::operator()(const BlackboardItem& item) const noexcept
{
    if (BlackboardKey::GetKeyLo(item.Key) == 0)
    {
        return false;
    }

    uint32 filterLo = BlackboardKey::GetKeyLo(Filter);
    uint32 itemLo = BlackboardKey::GetKeyLo(item.Key);
    if (filterLo != IgnoreKey && filterLo != itemLo)
    {
        return false;
    }

    uint32 filterHi = BlackboardKey::GetKeyHi(Filter);
    uint32 itemHi = BlackboardKey::GetKeyHi(item.Key);
    if (filterHi != IgnoreKey && filterHi != itemHi)
    {
        return false;
    }

    uint8 filterType = BlackboardKey::GetKeyType(Filter);
    uint8 itemType = BlackboardKey::GetKeyType(item.Key);
    if (filterType != IgnoreType && filterType != itemType)
    {
        return false;
    }

    return true;
}

void FixedBlackboard::Construct(BlockBufferAllocator& allocator, const Config& config)
{
    Storage.Construct(allocator, config.Capacity);
}

BlockBufferLayout FixedBlackboard::StaticLayout(const Config& config)
{
    return BlockBufferLayout::For<FixedBlackboard>().Container<TStorage>(config.Capacity);
}

uint32 FixedBlackboard::GetCapacity() const
{
    return Storage.GetCapacity();
}

FixedBlackboard::TItem* FixedBlackboard::GetData()
{
    return Storage.GetData();
}

const FixedBlackboard::TItem* FixedBlackboard::GetData() const
{
    return Storage.GetData();
}

void FixedBlackboard::Sort()
{
    Storage.Sort();
}

uint32 FixedBlackboard::GetNum() const
{
    return Storage.GetNum();
}

uint32 FixedBlackboard::GetNumValidItems() const
{
    return Storage.GetNumValidItems();
}

bool FixedBlackboard::HasValue(const BlackboardQuery& query) const
{
    return FindItemWithQuery(query) != nullptr;
}

bool FixedBlackboard::SetValue(blackboard_key_t key, blackboard_value_t value)
{
    if (BlackboardItem* item = FindItemWithQuery(key))
    {
        item->Value = value;
        return true;
    }

    return InsertKVP(key, value);
}

bool FixedBlackboard::GetValue(const BlackboardQuery& query, blackboard_value_t& outValue) const
{
    PHX_PROFILE_ZONE_SCOPED;

    const BlackboardItem* item = FindItemWithQuery(query);
    if (!item)
    {
        return false;
    }

    outValue = item->Value;
    return true;
}

bool FixedBlackboard::GetValue(blackboard_key_t key, blackboard_value_t& outValue) const
{
    return GetValue(BlackboardQuery(key), outValue);
}

uint32 FixedBlackboard::RemoveAll(const BlackboardQuery& query)
{
    return RemoveValuesMatchingQueryInternal(query);
}

BlackboardItem* FixedBlackboard::FindItemWithQuery(const BlackboardQuery& query)
{
    uint32 index;
    BlackboardItem* item = FindFirstItemWithQuery(query, index);
    while (item && !query(*item))
    {
        item = FindNextItemWithQuery(query, index, index);
    }
    return item;
}

const BlackboardItem* FixedBlackboard::FindItemWithQuery(const BlackboardQuery& query) const
{
    uint32 index;
    const BlackboardItem* item = FindFirstItemWithQuery(query, index);
    while (item && !query(*item))
    {
        item = FindNextItemWithQuery(query, index, index);
    }
    return item;
}

BlackboardItem* FixedBlackboard::FindFirstItemWithQuery(const BlackboardQuery& query, uint32& outIndex)
{
    return Storage.GetFirstItem(query.Filter, outIndex, CompareKeyHi{});
}

const BlackboardItem* FixedBlackboard::FindFirstItemWithQuery(const BlackboardQuery& query, uint32& outIndex) const
{
    return Storage.GetFirstItem(query.Filter, outIndex, CompareKeyHi{});
}

BlackboardItem* FixedBlackboard::FindNextItemWithQuery(const BlackboardQuery& query, uint32 currIndex, uint32& outIndex)
{
    return Storage.GetNextItem(query.Filter, currIndex, outIndex, CompareKeyHi{});
}

const BlackboardItem* FixedBlackboard::FindNextItemWithQuery(const BlackboardQuery& query, uint32 currIndex, uint32& outIndex) const
{
    return Storage.GetNextItem(query.Filter, currIndex, outIndex, CompareKeyHi{});
}

uint32 FixedBlackboard::RemoveValuesMatchingQueryInternal(const BlackboardQuery& query)
{
    PHX_PROFILE_ZONE_SCOPED;
    return Storage.RemoveAll(query.Filter, query, CompareKeyHi{});
}

bool FixedBlackboard::InsertKVP(blackboard_key_t key, blackboard_value_t value)
{
    PHX_PROFILE_ZONE_SCOPED;
    return Storage.EmplaceBack(key, value);
}

BlackboardValues FixedBlackboard::Enumerate(uint32 keyHi) const
{
    return { this, keyHi };
}

BlackboardValues::BlackboardValues(const FixedBlackboard* set, const BlackboardQuery& query)
    : Owner(set)
    , Query(query)
{
}

BlackboardValues::KeyHiIter::KeyHiIter(
    const FixedBlackboard* owner,
    const BlackboardQuery& query)
    : Owner(owner)
    , Query(query)
{
    PHX_ASSERT(Owner);
}

BlackboardValues::KeyHiIter::KeyHiIter(
    const FixedBlackboard* owner,
    const BlackboardQuery& query,
    const BlackboardItem* item,
    uint32 index)
    : Owner(owner)
    , Query(query)
    , Item(item)
    , Index(index)
{
    PHX_ASSERT(Owner);
}

const BlackboardItem& BlackboardValues::KeyHiIter::operator*() const
{
    PHX_ASSERT(Item);
    return *Item;
}

BlackboardValues::KeyHiIter& BlackboardValues::KeyHiIter::operator++()
{
    Item = Owner->FindNextItemWithQuery(Query, Index, Index);
    return *this;
}

BlackboardValues::KeyHiIter BlackboardValues::begin() const
{
    uint32 index;
    const BlackboardItem* item = Owner->FindFirstItemWithQuery(Query, index);
    return KeyHiIter(Owner, Query, item, index);
}

BlackboardValues::KeyHiIter BlackboardValues::end() const
{
    return KeyHiIter(Owner, Query);
}
