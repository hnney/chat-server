#ifndef _LT_HEAP_H_
#define _LT_HEAP_H_

#include <vector>
#include <algorithm>

using namespace std;

template <typename T, typename CMP> class heap {
    public:
        heap();
        ~heap();

        size_t push(const T &element);
        void sort();
        T pop(void);
        size_t size(void);
        T top(void);

    private:
        vector <T> data_;
        size_t size_;
};

template <typename T, typename CMP> T heap<T,CMP>::top(void)
{
    return data_[0];
}

template <typename T, typename CMP> size_t heap<T,CMP>::push(const T &element)
{
    if (size_ >= data_.size()) {
        data_.push_back(element);
    }
    else {
        data_[size_] = element;
    }
    size_++;
    push_heap(data_.begin(), data_.begin()+size_, CMP());
    return size_;
}

template <typename T, typename CMP> void heap<T,CMP>::sort() {
    if (size_ > 0) {
        //make_heap(data_.begin(), data_.end(), CMP());
        make_heap(data_.begin(), data_.begin()+size_, CMP());
    }
}

template <typename T, typename CMP> T heap<T,CMP>::pop(void)
{
    pop_heap(data_.begin(), data_.begin()+size_, CMP());
    if (size_ > 0) {
        size_--;
    }
    return data_[size_];
}

template <typename T, typename CMP> heap<T,CMP>::heap()
{
    size_ = 0;
}

template <typename T, typename CMP> heap<T,CMP>::~heap()
{
}

template <typename T, typename CMP> size_t heap<T,CMP>::size()
{
    return size_;
}

#endif

