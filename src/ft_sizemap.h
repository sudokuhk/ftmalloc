#ifndef __FT_SIZEMAP_H__
#define __FT_SIZEMAP_H__

#include <stdio.h>
#include <stddef.h>

namespace ftmalloc
{
#if 1//defined(TCMALLOC_ALIGN_8BYTES)
    // Unless we force to use 8 bytes alignment we use an alignment of
    // at least 16 bytes to statisfy requirements for some SSE types.
    // Keep in mind when using the 16 bytes alignment you can have a space
    // waste due alignment of 25%. (eg malloc of 24 bytes will get 32 bytes)
    static const size_t kMinAlign = 8;
    // Number of classes created until reach page size 128.
    static const size_t kBaseClasses = 16;
#else
    static const size_t kMinAlign = 16;
    static const size_t kBaseClasses = 9;
#endif

    static const size_t kPageShift = 16;
    static const size_t kNumClasses = kBaseClasses + 73;

    static const size_t kMaxThreadCacheSize = 4 << 20;

    static const size_t kPageSize = 1 << kPageShift;
    static const size_t kMaxSize = 256 * 1024;
    static const size_t kAlignment = 8;
    static const size_t kLargeSizeClass = 0;

    int AlignmentForSize(size_t size);

    // Size-class information + mapping
    class CSizeMap
    {
    private:
        // Number of objects to move between a per-thread list and a central
        // list in one shot.  We want this to be not too small so we can
        // amortize the lock overhead for accessing the central list.  Making
        // it too big may temporarily cause unnecessary memory wastage in the
        // per-thread free list until the scavenger cleans up the list.
        int num_objects_to_move_[kNumClasses];

        //-------------------------------------------------------------------
        // Mapping from size to size_class and vice versa
        //-------------------------------------------------------------------

        // Sizes <= 1024 have an alignment >= 8.  So for such sizes we have an
        // array indexed by ceil(size/8).  Sizes > 1024 have an alignment >= 128.
        // So for these larger sizes we have an array indexed by ceil(size/128).
        //
        // We flatten both logical arrays into one physical array and use
        // arithmetic to compute an appropriate index.  The constants used by
        // ClassIndex() were selected to make the flattening work.
        //
        // Examples:
        //   Size       Expression                      Index
        //   -------------------------------------------------------
        //   0          (0 + 7) / 8                     0
        //   1          (1 + 7) / 8                     1
        //   ...
        //   1024       (1024 + 7) / 8                  128
        //   1025       (1025 + 127 + (120<<7)) / 128   129
        //   ...
        //   32768      (32768 + 127 + (120<<7)) / 128  376
        static const int kMaxSmallSize = 1024;
        static const size_t kClassArraySize =
            ((kMaxSize + 127 + (120 << 7)) >> 7) + 1;
        unsigned char class_array_[kClassArraySize];

        // Compute index of the class_array[] entry for a given size
        static inline size_t ClassIndex(int s) {
            // Use unsigned arithmetic to avoid unnecessary sign extensions.
            //ASSERT(0 <= s);
            //ASSERT(s <= kMaxSize);
            //if (LIKELY(s <= kMaxSmallSize)) {
            if ((s <= kMaxSmallSize)) {
                return (static_cast<size_t>(s) + 7) >> 3;
            }
            else {
                return (static_cast<size_t>(s) + 127 + (120 << 7)) >> 7;
            }
        }

        int NumMoveSize(size_t size);

        // Mapping from size class to max size storable in that class
        size_t class_to_size_[kNumClasses];

        // Mapping from size class to number of pages to allocate at a time
        size_t class_to_pages_[kNumClasses];

        static size_t FLAGS_tcmalloc_transfer_num_objects;

    public:
        // Constructor should do nothing since we rely on explicit Init()
        // call, which may or may not be called before the constructor runs.
        CSizeMap() { }

        static CSizeMap & GetInstance();

        // Initialize the mapping arrays
        void Init();

        inline int SizeClass(int size) {
            return class_array_[ClassIndex(size)];
        }

        // Get the byte-size for a specified class
        inline size_t ByteSizeForClass(size_t cl) {
            return class_to_size_[cl];
        }

        // Mapping from size class to max size storable in that class
        inline size_t class_to_size(size_t cl) {
            return class_to_size_[cl];
        }

        // Mapping from size class to number of pages to allocate at a time
        inline size_t class_to_pages(size_t cl) {
            return class_to_pages_[cl];
        }

        // Number of objects to move between a per-thread list and a central
        // list in one shot.  We want this to be not too small so we can
        // amortize the lock overhead for accessing the central list.  Making
        // it too big may temporarily cause unnecessary memory wastage in the
        // per-thread free list until the scavenger cleans up the list.
        inline int num_objects_to_move(size_t cl) {
            return num_objects_to_move_[cl];
        }

        void print();
    };
}

#endif //__SIZEMAP_H__