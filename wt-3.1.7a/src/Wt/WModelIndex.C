/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <iostream>
#include <cstring>
#include <boost/functional/hash.hpp>

#include "Wt/WModelIndex"
#include "Wt/WAbstractItemModel"
#include "WtException.h"

namespace Wt {

WModelIndex::WModelIndex()
  : model_(0),
    row_(0),
    column_(0),
    internalId_(0)
{ }

boost::any WModelIndex::data(int role) const
{
  return model_->data(*this, role);
}

WFlags<ItemFlag> WModelIndex::flags() const
{
  return model_->flags(*this);
}

WModelIndex WModelIndex::child(int row, int column) const
{
  return model_ ? model_->index(row, column, *this) : WModelIndex();
}

WModelIndex WModelIndex::parent() const
{
  return model_ ? model_->parent(*this) : WModelIndex();
}

WModelIndex WModelIndex::ancestor(int depth) const
{
  if (depth == 0)
    return *this;
  else
    return parent().ancestor(depth - 1);
}

int WModelIndex::depth() const
{
  if (isValid())
    return parent().depth() + 1;
  else
    return 0;
}

bool WModelIndex::operator== (const WModelIndex& other) const
{
  return model_ == other.model_ &&
    row_ == other.row_ &&
    column_ == other.column_ &&
    internalId_ == other.internalId_;
}

bool WModelIndex::operator!= (const WModelIndex& other) const
{
  return !(*this == other);
}

bool WModelIndex::operator< (const WModelIndex& i2) const
{
  const WModelIndex& i1 = *this;

  if (!i1.isValid())
    return i2.isValid();
  else if (!i2.isValid())
    return false;
  else if (i1 == i2)
    return false;
  else if (i1.model() != i2.model()) {
    std::cerr << "Comparing indexes from different models are you?"
	      << std::endl;
    return false;
  }

  int i1Depth = i1.depth();
  int i2Depth = i2.depth();
  unsigned e = std::min(i1Depth, i2Depth);

  WModelIndex a1 = i1.ancestor(i1Depth - e);
  WModelIndex a2 = i2.ancestor(i2Depth - e);

  if (a1 == a2)
    return i1Depth < i2Depth;

  for (unsigned i = e; i > 0; --i) {
    WModelIndex p1 = a1.parent();
    WModelIndex p2 = a2.parent();

    if (p1 == p2) {
      if (a1.row() < a2.row())
	return true;
      else if (a1.row() > a2.row())
	return false;
      else if (a1.column() < a2.column())
	return true;
      else
	return false;
    }

    a1 = p1;
    a2 = p2;
  }

  return false; // unreachable code
}

bool WModelIndex::UnorderedLess::operator() (const WModelIndex& i1,
					     const WModelIndex& i2) const
{
  if (!i1.isValid())
    return i2.isValid();
  else if (!i2.isValid())
    return false;
  else if (i1 == i2)
    return false;
  else if (i1.model() != i2.model()) {
    std::cerr << "Comparing indexes from different models are you?"
	      << std::endl;
    return false;
  } else if (i1.row() < i2.row())
    return true;
  else if (i1.row() > i2.row())
    return false;
  else if (i1.column() < i2.column())
    return true;
  else if (i1.column() > i2.column())
    return false;
  else
    return i1.internalId_ < i2.internalId_;
}

WModelIndex::WModelIndex(int row, int column, const WAbstractItemModel *model,
			 void *ptr)
  : model_(model),
    row_(row),
    column_(column),
    internalId_(reinterpret_cast< ::uint64_t >(ptr))
{ }

WModelIndex::WModelIndex(int row, int column, const WAbstractItemModel *model,
			 ::uint64_t id)
  : model_(model),
    row_(row),
    column_(column),
    internalId_(id)
{ }

std::size_t hash_value(const Wt::WModelIndex& index) {
  boost::hash<int> intHasher;
  boost::hash< ::uint64_t > longHasher;

  return intHasher(index.row()) + intHasher(index.column())
    + longHasher(index.internalId());
}

void WModelIndex::encodeAsRawIndex()
{
  if (model_) {
    if (isRawIndex())
      throw WtException("WModelIndex::encodeAsRawIndex(): "
			"cannot encode a raw index to raw again");

    internalId_ = reinterpret_cast< ::uint64_t >(model_->toRawIndex(*this));
    row_ = column_ = -42;
  }
}

WModelIndex WModelIndex::decodeFromRawIndex() const
{
  if (model_) {
    if (!isRawIndex())
      throw WtException("WModelIndex::decodeFromRawIndex(): "
			"can only decode an encoded raw index");

    return model_->fromRawIndex(internalPointer());
  } else
    return *this;
}

bool WModelIndex::isRawIndex() const
{
  return row_ == -42 && column_ == -42;
}

void WModelIndex::encodeAsRawIndexes(WModelIndexSet& indexes)
{
  for (WModelIndexSet::iterator i = indexes.begin(); i != indexes.end(); ++i)
    (const_cast<WModelIndex &>(*i)).encodeAsRawIndex();
}

WModelIndexSet
WModelIndex::decodeFromRawIndexes(const WModelIndexSet& encodedIndexes)
{
  WModelIndexSet result;

  for (WModelIndexSet::const_iterator i = encodedIndexes.begin();
       i != encodedIndexes.end(); ++i) {
    WModelIndex n = i->decodeFromRawIndex();
    if (n.isValid())
      result.insert(n);
  }

  return result;
}

bool WModelIndex::isAncestor(const Wt::WModelIndex& i1,
			     const Wt::WModelIndex& i2) {
  if (!i1.isValid())
    return false;

  for (Wt::WModelIndex p = i1.parent(); p.isValid(); p = p.parent()) {
    if (p == i2)
      return true;
  }

  return !i2.isValid();
}

}
