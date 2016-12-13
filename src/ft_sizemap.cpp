#include "ft_sizemap.h"
#include "ft_malloc_log.h"

namespace ftmalloc
{
    size_t CSizeMap::FLAGS_tcmalloc_transfer_num_objects = kPageSize >> 3;

    // Note: the following only works for "n"s that fit in 32-bits, but
    // that is fine since we only use it for small sizes.
    static inline int LgFloor(size_t n) {
        int log = 0;
        for (int i = 4; i >= 0; --i) {
            int shift = (1 << i);
            size_t x = n >> shift;
            if (x != 0) {
                n = x;
                log += shift;
            }
        }
        //ASSERT(n == 1);
        return log;
    }

    int AlignmentForSize(size_t size) {
        int alignment = kAlignment;
        if (size > kMaxSize) {
            // Cap alignment at kPageSize for large sizes.
            alignment = kPageSize;
        }
        else if (size >= 128) {
            // Space wasted due to alignment is at most 1/8, i.e., 12.5%.
            alignment = (1 << LgFloor(size)) / 8;
        }
        else if (size >= kMinAlign) {
            // We need an alignment of at least 16 bytes to satisfy
            // requirements for some SSE types.
            alignment = kMinAlign;
        }
        // Maximum alignment allowed is page size alignment.
        if (alignment > kPageSize) {
            alignment = kPageSize;
        }
        //CHECK_CONDITION(size < kMinAlign || alignment >= kMinAlign);
        //CHECK_CONDITION((alignment & (alignment - 1)) == 0);
        return alignment;
    }

    CSizeMap & CSizeMap::GetInstance()
    {
        static CSizeMap sInstance;
        static bool bInit = false;
        if (!bInit) {
            bInit = true;
            sInstance.Init();
            sInstance.print();
        }
        
        return sInstance;
    }

    // Initialize the mapping arrays
    void CSizeMap::Init() {
        //InitTCMallocTransferNumObjects();

        // Do some sanity checking on add_amount[]/shift_amount[]/class_array[]
        if (ClassIndex(0) != 0) {
        }
        if (ClassIndex(kMaxSize) >= sizeof(class_array_)) {
        }

        // Compute the size classes we want to use
        int sc = 1;   // Next size class to assign
        int alignment = kAlignment;
        //CHECK_CONDITION(kAlignment <= kMinAlign);
        for (size_t size = kAlignment; size <= kMaxSize; size += alignment) {
            alignment = AlignmentForSize(size);
            //CHECK_CONDITION((size % alignment) == 0);

            int blocks_to_move = NumMoveSize(size) / 4;
            size_t psize = 0;
            do {
                psize += kPageSize;
                // Allocate enough pages so leftover is less than 1/8 of total.
                // This bounds wasted space to at most 12.5%.
                while ((psize % size) > (psize >> 3)) {
                    psize += kPageSize;
                }
                // Continue to add pages until there are at least as many objects in
                // the span as are needed when moving objects from the central
                // freelists and spans to the thread caches.
            } while ((psize / size) < (blocks_to_move));
            const size_t my_pages = psize >> kPageShift;

            if (sc > 1 && my_pages == class_to_pages_[sc - 1]) {
                // See if we can merge this into the previous class without
                // increasing the fragmentation of the previous class.
                const size_t my_objects = (my_pages << kPageShift) / size;
                const size_t prev_objects = (class_to_pages_[sc - 1] << kPageShift)
                    / class_to_size_[sc - 1];
                if (my_objects == prev_objects) {
                    // Adjust last class to include this size
                    class_to_size_[sc - 1] = size;
                    continue;
                }
            }

            // Add new class
            class_to_pages_[sc] = my_pages;
            class_to_size_[sc] = size;
            sc++;
        }
        if (sc != kNumClasses) {
        }

        // Initialize the mapping arrays
        int next_size = 0;
        for (int c = 1; c < kNumClasses; c++) {
            const int max_size_in_class = class_to_size_[c];
            for (int s = next_size; s <= max_size_in_class; s += kAlignment) {
                class_array_[ClassIndex(s)] = c;
            }
            next_size = max_size_in_class + kAlignment;
        }

        // Double-check sizes just to be safe
        for (size_t size = 0; size <= kMaxSize;) {
            const int sc = SizeClass(size);
            if (sc <= 0 || sc >= kNumClasses) {
            }
            if (sc > 1 && size <= class_to_size_[sc - 1]) {
            }
            const size_t s = class_to_size_[sc];
            if (size > s || s == 0) {
            }
            if (size <= kMaxSmallSize) {
                size += 8;
            }
            else {
                size += 128;
            }
        }

        // Initialize the num_objects_to_move array.
        for (size_t cl = 1; cl < kNumClasses; ++cl) {
            num_objects_to_move_[cl] = class_to_pages_[cl] * kPageSize / class_to_size_[cl];
            //num_objects_to_move_[cl] = NumMoveSize(ByteSizeForClass(cl));
        }
    }

    int CSizeMap::NumMoveSize(size_t size) {
        if (size == 0) return 0;
        // Use approx 64k transfers between thread and central caches.
        int num = static_cast<int>(64.0 * 1024.0 / size);
        if (num < 2) num = 2;

        // Avoid bringing too many objects into small object free lists.
        // If this value is too large:
        // - We waste memory with extra objects sitting in the thread caches.
        // - The central freelist holds its lock for too long while
        //   building a linked list of objects, slowing down the allocations
        //   of other threads.
        // If this value is too small:
        // - We go to the central freelist too often and we have to acquire
        //   its lock each time.
        // This value strikes a balance between the constraints above.

#if 1
        if (num > FLAGS_tcmalloc_transfer_num_objects)
            num = FLAGS_tcmalloc_transfer_num_objects;
#else

#endif
        return num;
    }

    void CSizeMap::print()
    {
        size_t length = sizeof(class_to_size_) / sizeof(class_to_size_[0]);
        for (size_t i = 0; i < length; i++) {
            FT_LOG(FT_DEBUG, "class_to_size_[%zd] = %zd, class_to_pages_[%zd] = %zd, number:%d",
                i, class_to_size_[i], i, class_to_pages_[i], num_objects_to_move_[i]);
        }
    }
}
