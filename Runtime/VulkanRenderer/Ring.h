// AMD Cauldron code
//
// Copyright(c) 2020 Advanced Micro Devices, Inc.All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once
#include <cassert>
#include <cstdint>
#include "Foundation/NonCopyable.h"
// This is the typical ring buffer, it is used by resources that will be reused.
// For example, commandlists and 'dynamic' constant buffers, etc..

namespace vkgfx {

class Ring {
public:
    void Create(uint32_t TotalSize) {
        _head = 0;
        _allocatedSize = 0;
        _totalSize = TotalSize;
    }

    uint32_t GetSize() const {
        return _allocatedSize;
    }
    uint32_t GetHead() const {
        return _head;
    }
    uint32_t GetTail() const {
        return (_head + _allocatedSize) % _totalSize;
    }

    //helper to avoid allocating chunks that wouldn't fit contiguously in the ring
    uint32_t PaddingToAvoidCrossOver(uint32_t size) {
        int tail = GetTail();
        if ((tail + size) > _totalSize) {
            return (_totalSize - tail);
        }
        return 0;
    }

    bool Alloc(uint32_t size, uint32_t *pOut) {
        if (_allocatedSize + size <= _totalSize) {
            if (pOut)
                *pOut = GetTail();

            _allocatedSize += size;
            return true;
        }

        assert(false);
        return false;
    }

    bool Free(uint32_t size) {
        if (_allocatedSize >= size) {
            _head = (_head + size) % _totalSize;
            _allocatedSize -= size;
            return true;
        }
        return false;
    }

    size_t GetAllocatableSize() const {
	    return _totalSize - _allocatedSize;
    }
private:
    uint32_t _head = 0;
    uint32_t _totalSize = 0;
    uint32_t _allocatedSize = 0;
};

//
// This class can be thought as ring buffer inside a ring buffer. The outer ring is for ,
// the frames and the internal one is for the resources that were allocated for that frame.
// The size of the outer ring is typically the number of back buffers.
//
// When the outer ring is full, for the next allocation it automatically frees the entries
// of the oldest frame and makes those entries available for the next frame. This happens
// when you call 'OnBeginFrame()'
//
class RingWithTabs : public NonCopyable {
public:
    void OnCreate(uint32_t numberOfBackBuffers, uint32_t memTotalSize) {
        _backBufferIndex = 0;
        _numberOfBackBuffers = numberOfBackBuffers;

        //init mem per frame tracker
        _memAllocatedInFrame = 0;
        for (int i = 0; i < 4; i++) {
            _allocatedMemPerBackBuffer[i] = 0;
        }
        _mem.Create(memTotalSize);
    }

    void OnDestroy() {
        _mem.Free(_mem.GetSize());
    }

    bool Alloc(uint32_t size, uint32_t *pOut) {
        uint32_t padding = _mem.PaddingToAvoidCrossOver(size);
        if (padding > 0) {
            _memAllocatedInFrame += padding;

            if (_mem.Alloc(padding, NULL) == false)    //alloc chunk to avoid crossover, ignore offset
            {
                return false;    //no mem, cannot allocate apdding
            }
        }

        if (_mem.Alloc(size, pOut) == true) {
            _memAllocatedInFrame += size;
            return true;
        }
        return false;
    }

    void OnBeginFrame() {
        _allocatedMemPerBackBuffer[_backBufferIndex] = _memAllocatedInFrame;
        _memAllocatedInFrame = 0;

        _backBufferIndex = (_backBufferIndex + 1) % _numberOfBackBuffers;

        // free all the entries for the oldest buffer in one go
        uint32_t memToFree = _allocatedMemPerBackBuffer[_backBufferIndex];
        _mem.Free(memToFree);
    }

    size_t GetAllocatableSize() const {
	    return _mem.GetAllocatableSize();
    }
private:
    //internal ring buffer
    Ring _mem;
    //this is the external ring buffer (I could have reused the Ring class though)
    uint32_t _backBufferIndex = 0;
    uint32_t _memAllocatedInFrame = 0;
    uint32_t _numberOfBackBuffers = 0;
    uint32_t _allocatedMemPerBackBuffer[4] = {0};
};

}    // namespace vkgfx