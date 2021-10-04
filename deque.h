#pragma once

#include <initializer_list>
#include <algorithm>
#include <deque>
#include <memory>

class Deque {
public:
  Deque() {
    data_ = std::make_unique<std::unique_ptr<int[]>[]>(1);
    data_[0] = std::make_unique<int[]>(block_size_);
    allocd_ = block_size_;
    tail_ = head_ = 0;
    empty_ = true;
  };
  Deque(const Deque& rhs) {
    head_ = rhs.head_;
    tail_ = rhs.tail_;
    allocd_ = rhs.allocd_;
    empty_ = rhs.empty_;
    data_ = std::make_unique<std::unique_ptr<int[]>[]>(BlockId(allocd_));
    for (size_t i = 0; i < BlockId(allocd_); ++i) {
      data_[i] = std::make_unique<int[]>(block_size_);
    }
    for (size_t i = 0; i < rhs.Size(); ++i) {
      (*this)[i] = rhs[i];
    }
  }
  Deque(Deque&& rhs) {
    Swap(rhs);
  }
  explicit Deque(size_t size) {
    size_t block_count = (size + block_size_ - 1) / block_size_;
    data_ = std::make_unique<std::unique_ptr<int[]>[]>(block_count);
    allocd_ = block_count * block_size_;
    empty_ = (size == 0);
    for (size_t i = 0; i < BlockId(allocd_); ++i) {
      data_[i] = std::make_unique<int[]>(block_size_);
    }
    head_ = 0;
    tail_ = size;
    for (size_t i = 0; i < size; ++i) {
      data_[BlockId(i)][BlockPos(i)] = 0;
    }
  }

  Deque(std::initializer_list<int> list) {
    Deque cur(list.size());
    auto it = list.begin();
    for (size_t i = 0; i < list.size(); ++i, ++it) {
      cur[i] = *it;
    }
    Swap(cur);
  }

  Deque& operator=(Deque rhs) {
    Swap(rhs);
    return *this;
  }

  void Swap(Deque& rhs) {
    if (data_ != rhs.data_) {
      std::swap(data_, rhs.data_);
      std::swap(head_, rhs.head_);
      std::swap(tail_, rhs.tail_);
      std::swap(allocd_, rhs.allocd_);
      std::swap(empty_, rhs.empty_);
    }
  }

  void PushBack(int value) {
    if (tail_ == allocd_ && head_ < block_size_) {
      size_t block_count = BlockId(allocd_);
      auto new_data = std::make_unique<std::unique_ptr<int[]>[]>(block_count + 1);
      for (size_t i = 0; i < block_count; ++i) {
        new_data[i].swap(data_[i]);
      }
      new_data[block_count] = std::make_unique<int[]>(block_size_);
      new_data[block_count][0] = value;
      std::swap(data_, new_data);
      tail_ = allocd_ + 1;
      allocd_ += block_size_;
    } else if (BlockPos(tail_) == 0 && BlockId(tail_) == BlockId(head_) && tail_ <= head_ &&
               !empty_) {
      size_t block_count = BlockId(allocd_);
      auto new_data = std::make_unique<std::unique_ptr<int[]>[]>(block_count + 1);
      for (size_t i = 0; i < BlockId(tail_); ++i) {
        new_data[i].swap(data_[i]);
      }
      new_data[BlockId(tail_)] = std::make_unique<int[]>(block_size_);
      new_data[BlockId(tail_)][0] = value;
      for (size_t i = BlockId(tail_) + 1; i <= block_count; ++i) {
        new_data[i].swap(data_[i - 1]);
      }
      std::swap(data_, new_data);
      ++tail_;
      head_ += block_size_;
      allocd_ += block_size_;
    } else {
      tail_ = (tail_ % allocd_) + 1;
      data_[BlockId(tail_ - 1)][BlockPos(tail_ - 1)] = value;
    }
    empty_ = false;
  }

  void PopBack() {
    if (Size() == 0) {
      return;
    } else if (tail_ == 1 && head_ != 0) {
      tail_ = allocd_;
    } else {
      --tail_;
    }
    if (head_ == tail_) {
      empty_ = true;
    }
  }

  void PushFront(int value) {
    size_t new_head_block = (head_ == 0 ? BlockId(allocd_) - 1 : BlockId(head_ - 1));
    if (head_ == 0 && (BlockId(tail_ - 1) == BlockId(allocd_) - 1 || empty_)) {
      size_t block_count = BlockId(allocd_);
      auto new_data = std::make_unique<std::unique_ptr<int[]>[]>(block_count + 1);
      new_data[0] = std::make_unique<int[]>(block_size_);
      new_data[0][block_size_ - 1] = value;
      for (size_t i = 1; i <= block_count; ++i) {
        new_data[i].swap(data_[i - 1]);
      }
      std::swap(data_, new_data);
      allocd_ += block_size_;
      tail_ += block_size_;
      head_ = block_size_ - 1;
    } else if ((head_ > tail_ || (head_ == tail_ && !empty_)) &&
               new_head_block == BlockId(tail_ - 1)) {
      size_t block_count = BlockId(allocd_);
      auto new_data = std::make_unique<std::unique_ptr<int[]>[]>(block_count + 1);
      ++new_head_block;
      for (size_t i = 0; i < new_head_block; ++i) {
        new_data[i].swap(data_[i]);
      }
      new_data[new_head_block] = std::make_unique<int[]>(block_size_);
      new_data[new_head_block][block_size_ - 1] = value;
      for (size_t i = new_head_block + 1; i <= block_count; ++i) {
        new_data[i].swap(data_[i - 1]);
      }
      std::swap(data_, new_data);
      allocd_ += block_size_;
      head_ = (new_head_block + 1) * block_size_ - 1;
    } else {
      head_ = (head_ == 0 ? allocd_ - 1 : head_ - 1);
      data_[BlockId(head_)][BlockPos(head_)] = value;
    }
    empty_ = false;
  }

  void PopFront() {
    if (Size() == 0) {
      return;
    } else {
      head_ = (head_ + 1) % allocd_;
    }
    if (head_ == tail_) {
      empty_ = true;
    }
  }

  int& operator[](size_t ind) {
    if (empty_) {
      throw std::range_error("Out of bounds");
    }
    size_t pos = (head_ + ind) % allocd_;
    return data_[BlockId(pos)][BlockPos(pos)];
  }

  int operator[](size_t ind) const {
    if (empty_) {
      throw std::range_error("Out of bounds");
    }
    size_t pos = (head_ + ind) % allocd_;
    return data_[BlockId(pos)][BlockPos(pos)];
  }

  size_t Size() const {
    if (head_ < tail_) {
      return tail_ - head_;
    } else if (empty_) {
      return 0;
    } else if (head_ == tail_) {
      return allocd_;
    } else {
      return allocd_ - head_ + tail_;
    }
  }

  void Clear() {
    *this = Deque();
  }

private:
  const size_t block_size_ = 128;
  size_t allocd_ = 0;
  size_t head_ = 0;
  size_t tail_ = 0;
  bool empty_ = true;
  std::unique_ptr<std::unique_ptr<int[]>[]> data_ = nullptr;

  size_t BlockId(size_t ind) const {
    if (ind > allocd_) {
      return allocd_ / block_size_;
    } else {
      return ind / block_size_;
    }
  }
  size_t BlockPos(size_t ind) const {
    return ind % block_size_;
  }
};
