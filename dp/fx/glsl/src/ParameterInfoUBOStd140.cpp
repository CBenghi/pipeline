// Copyright (c) 2012-2016, NVIDIA CORPORATION. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include <ParameterInfoUBOStd140.h>
#include <dp/fx/ParameterGroupLayout.h>

namespace dp
{
  namespace fx
  {
    namespace glsl
    {
      namespace
      {


        /************************************************************************/
        /* vecnt -> vecmt upcast with conversion                                */
        /************************************************************************/
        template <typename SrcType, typename DstType, int numSrcComponents, int numDstComponents>
        class ParameterInfoVectorConversion : public ParameterGroupLayout::ParameterInfo
        {
        public:
          ParameterInfoVectorConversion( size_t& offset, size_t arraySize );
          virtual void convert( void* dstBase, const void *src ) const;
          size_t getAlignment() const;
          size_t getSize() const;
        private:
          size_t m_arraySize;
          size_t m_offset;
        };

        template <typename SrcType, typename DstType, int numSrcComponents, int numDstComponents>
        ParameterInfoVectorConversion<SrcType, DstType, numSrcComponents, numDstComponents>::ParameterInfoVectorConversion( size_t& offset, size_t arraySize )
          : m_arraySize(arraySize)
        {
          DP_STATIC_ASSERT(numSrcComponents <= numDstComponents);

          size_t alignment = getAlignment();
          m_offset = ((offset + alignment - 1) / alignment ) * alignment;

          // return next available offset
          offset = m_offset + getSize();
        }

        template <typename SrcType, typename DstType, int numSrcComponents, int numDstComponents>
        void ParameterInfoVectorConversion<SrcType, DstType, numSrcComponents, numDstComponents>::convert( void* dstBase, void const*src ) const
        {
          char* tmpDst = reinterpret_cast<char*>(dstBase) + m_offset;
          SrcType const* srcType = reinterpret_cast<SrcType const*>(src);
          if ( !m_arraySize )
          {
            DstType* dstType = reinterpret_cast<DstType*>(tmpDst);
            for ( size_t index = 0;index < numSrcComponents;++index )
            {
              *(dstType + index) = *(srcType + index);
            }
          }
          else
          {
            DP_ASSERT( !"path untested" );

            size_t dstStride = sizeof(DstType) * 4;
            for ( size_t arrayIndex = 0; arrayIndex < m_arraySize; ++arrayIndex )
            {
              DstType* dstType = reinterpret_cast<DstType*>(tmpDst);
              for ( size_t index = 0;index < numSrcComponents;++index )
              {
                *(dstType + index) = *(srcType + index);
              }
              tmpDst += dstStride;
            }
          }
        }

        template <typename SrcType, typename DstType, int numSrcComponents, int numDstComponents>
        size_t ParameterInfoVectorConversion<SrcType, DstType, numSrcComponents, numDstComponents>::getAlignment() const
        {
          if ( !m_arraySize )
          {
            return sizeof(DstType) * numDstComponents;
          }
          else
          {
            DP_ASSERT( !"path not tested" );
            return sizeof(DstType) * 4 * m_arraySize;
          }
        }

        template <typename SrcType, typename DstType, int numSrcComponents, int numDstComponents>
        size_t ParameterInfoVectorConversion<SrcType, DstType, numSrcComponents, numDstComponents>::getSize() const
        {
          if ( !m_arraySize )
          {
            return sizeof(DstType) * numSrcComponents;
          }
          else
          {
            DP_ASSERT( !"path not tested" );
            return sizeof(DstType) * 4 * m_arraySize;
          }
        }

        /************************************************************************/
        /* matmnt -> vecmnt upcast with conversion                              */
        /************************************************************************/
        template <typename SrcType, typename DstType, int numSrcComponents, int numDstComponents, int numRows>
        class ParameterInfoMatrixConversion : public ParameterGroupLayout::ParameterInfo
        {
        public:
          ParameterInfoMatrixConversion( size_t& offset, size_t arraySize );
          virtual void convert( void* dstBase, const void *src ) const;
          size_t getAlignment() const;
          size_t getSize() const;
        private:
          size_t m_arraySize;
          size_t m_offset;
        };

        template <typename SrcType, typename DstType, int numSrcComponents, int numDstComponents, int numRows>
        ParameterInfoMatrixConversion<SrcType, DstType, numSrcComponents, numDstComponents, numRows>::ParameterInfoMatrixConversion( size_t& offset, size_t arraySize )
          : m_arraySize( arraySize )
        {
          DP_STATIC_ASSERT(numSrcComponents <= numDstComponents);

          size_t alignment = getAlignment();
          m_offset = ((offset + alignment - 1) / alignment ) * alignment;
          offset = m_offset + getSize();
        }

        template <typename SrcType, typename DstType, int numSrcComponents, int numDstComponents, int numRows>
        void ParameterInfoMatrixConversion<SrcType, DstType, numSrcComponents, numDstComponents, numRows>::convert( void* dstBase, void const*src ) const
        {
          if ( !m_arraySize )
          {
            char* tmpDst = reinterpret_cast<char*>(dstBase) + m_offset;
            DstType* dstType = reinterpret_cast<DstType*>(tmpDst);
            SrcType const* srcType = reinterpret_cast<SrcType const*>(src);
            for ( size_t row = 0;row < numRows;++row )
            {
              for ( size_t index = 0;index < numSrcComponents;++index )
              {
                *(dstType + index) = *(srcType + index);
              }
              srcType += numSrcComponents;
              dstType += 4;
            }
          }
          else
          {
            DP_ASSERT( !"path untested" );
            size_t stride = sizeof(DstType) * 4;
            SrcType const* srcType = reinterpret_cast<SrcType const*>(src);
            for ( size_t arrayIndex = 0;arrayIndex < m_arraySize;++arrayIndex )
            {
              char* tmpDst = reinterpret_cast<char*>(dstBase) + m_offset;
              for ( size_t row = 0;row < numRows;++row )
              {
                DstType* dstType = reinterpret_cast<DstType*>(tmpDst);
                for ( size_t index = 0;index < numSrcComponents;++index )
                {
                  *(dstType + index) = *(srcType + index);
                }
                srcType += numSrcComponents;
                tmpDst += stride;
              }
            }
          }
        }

        template <typename SrcType, typename DstType, int numSrcComponents, int numDstComponents, int numRows>
        size_t ParameterInfoMatrixConversion<SrcType, DstType, numSrcComponents, numDstComponents, numRows>::getAlignment() const
        {
          return 4 * sizeof(DstType);
        }

        template <typename SrcType, typename DstType, int numSrcComponents, int numDstComponents, int numRows>
        size_t ParameterInfoMatrixConversion<SrcType, DstType, numSrcComponents, numDstComponents, numRows>::getSize() const
        {
          if ( !m_arraySize )
          {
            return 4 * numRows * sizeof(DstType);
          }
          else
          {
            return 4 * numRows * sizeof(DstType) * m_arraySize;
          }
        }

      } // namespace anonymous

      /************************************************************************/
      /* \brief offset is in/out, next free offset location                   */
      /************************************************************************/
      dp::fx::ParameterGroupLayout::ParameterInfoSharedPtr createParameterInfoUBOStd140( unsigned int type, size_t& offset, size_t arraySize )
      {
        switch (type)
        {
          case PT_FLOAT32                : return( std::make_shared<ParameterInfoVectorConversion<dp::Float32, dp::Float32, 1, 1>>( offset, arraySize ) );
          case PT_FLOAT32 | PT_VECTOR2   : return( std::make_shared<ParameterInfoVectorConversion<dp::Float32, dp::Float32, 2, 2>>( offset, arraySize ) );
          case PT_FLOAT32 | PT_VECTOR3   : return( std::make_shared<ParameterInfoVectorConversion<dp::Float32, dp::Float32, 3, 4>>( offset, arraySize ) );
          case PT_FLOAT32 | PT_VECTOR4   : return( std::make_shared<ParameterInfoVectorConversion<dp::Float32, dp::Float32, 4, 4>>( offset, arraySize ) );
          case PT_INT8                   : return( std::make_shared<ParameterInfoVectorConversion<int8_t     , int32_t    , 1, 1>>( offset, arraySize ) );
          case PT_INT8 | PT_VECTOR2      : return( std::make_shared<ParameterInfoVectorConversion<int8_t     , int32_t    , 2, 2>>( offset, arraySize ) );
          case PT_INT8 | PT_VECTOR3      : return( std::make_shared<ParameterInfoVectorConversion<int8_t     , int32_t    , 3, 4>>( offset, arraySize ) );
          case PT_INT8 | PT_VECTOR4      : return( std::make_shared<ParameterInfoVectorConversion<int8_t     , int32_t    , 4, 4>>( offset, arraySize ) );
          case PT_INT16                  : return( std::make_shared<ParameterInfoVectorConversion<int16_t    , int32_t    , 1, 1>>( offset, arraySize ) );
          case PT_INT16 | PT_VECTOR2     : return( std::make_shared<ParameterInfoVectorConversion<int16_t    , int32_t    , 2, 2>>( offset, arraySize ) );
          case PT_INT16 | PT_VECTOR3     : return( std::make_shared<ParameterInfoVectorConversion<int16_t    , int32_t    , 3, 4>>( offset, arraySize ) );
          case PT_INT16 | PT_VECTOR4     : return( std::make_shared<ParameterInfoVectorConversion<int16_t    , int32_t    , 4, 4>>( offset, arraySize ) );
          case PT_INT32                  : return( std::make_shared<ParameterInfoVectorConversion<int32_t    , int32_t    , 1, 1>>( offset, arraySize ) );
          case PT_INT32 | PT_VECTOR2     : return( std::make_shared<ParameterInfoVectorConversion<int32_t    , int32_t    , 2, 2>>( offset, arraySize ) );
          case PT_INT32 | PT_VECTOR3     : return( std::make_shared<ParameterInfoVectorConversion<int32_t    , int32_t    , 3, 4>>( offset, arraySize ) );
          case PT_INT32 | PT_VECTOR4     : return( std::make_shared<ParameterInfoVectorConversion<int32_t    , int32_t    , 4, 4>>( offset, arraySize ) );
          case PT_INT64                  : return( std::make_shared<ParameterInfoVectorConversion<int64_t    , int64_t    , 1, 1>>( offset, arraySize ) );
          case PT_INT64 | PT_VECTOR2     : return( std::make_shared<ParameterInfoVectorConversion<int64_t    , int64_t    , 2, 2>>( offset, arraySize ) );
          case PT_INT64 | PT_VECTOR3     : return( std::make_shared<ParameterInfoVectorConversion<int64_t    , int64_t    , 3, 4>>( offset, arraySize ) );
          case PT_INT64 | PT_VECTOR4     : return( std::make_shared<ParameterInfoVectorConversion<int64_t    , int64_t    , 4, 4>>( offset, arraySize ) );
          case PT_UINT8                  : return( std::make_shared<ParameterInfoVectorConversion<uint8_t    , uint32_t   , 1, 1>>( offset, arraySize ) );
          case PT_UINT8 | PT_VECTOR2     : return( std::make_shared<ParameterInfoVectorConversion<uint8_t    , uint32_t   , 2, 2>>( offset, arraySize ) );
          case PT_UINT8 | PT_VECTOR3     : return( std::make_shared<ParameterInfoVectorConversion<uint8_t    , uint32_t   , 3, 4>>( offset, arraySize ) );
          case PT_UINT8 | PT_VECTOR4     : return( std::make_shared<ParameterInfoVectorConversion<uint8_t    , uint32_t   , 4, 4>>( offset, arraySize ) );
          case PT_UINT16                 : return( std::make_shared<ParameterInfoVectorConversion<uint16_t   , uint32_t   , 1, 1>>( offset, arraySize ) );
          case PT_UINT16 | PT_VECTOR2    : return( std::make_shared<ParameterInfoVectorConversion<uint16_t   , uint32_t   , 2, 2>>( offset, arraySize ) );
          case PT_UINT16 | PT_VECTOR3    : return( std::make_shared<ParameterInfoVectorConversion<uint16_t   , uint32_t   , 3, 4>>( offset, arraySize ) );
          case PT_UINT16 | PT_VECTOR4    : return( std::make_shared<ParameterInfoVectorConversion<uint16_t   , uint32_t   , 4, 4>>( offset, arraySize ) );
          case PT_UINT32                 : return( std::make_shared<ParameterInfoVectorConversion<uint32_t   , uint32_t   , 1, 1>>( offset, arraySize ) );
          case PT_UINT32 | PT_VECTOR2    : return( std::make_shared<ParameterInfoVectorConversion<uint32_t   , uint32_t   , 2, 2>>( offset, arraySize ) );
          case PT_UINT32 | PT_VECTOR3    : return( std::make_shared<ParameterInfoVectorConversion<uint32_t   , uint32_t   , 3, 4>>( offset, arraySize ) );
          case PT_UINT32 | PT_VECTOR4    : return( std::make_shared<ParameterInfoVectorConversion<uint32_t   , uint32_t   , 4, 4>>( offset, arraySize ) );
          case PT_UINT64                 : return( std::make_shared<ParameterInfoVectorConversion<uint64_t   , uint64_t   , 1, 1>>( offset, arraySize ) );
          case PT_UINT64 | PT_VECTOR2    : return( std::make_shared<ParameterInfoVectorConversion<uint64_t   , uint64_t   , 2, 2>>( offset, arraySize ) );
          case PT_UINT64 | PT_VECTOR3    : return( std::make_shared<ParameterInfoVectorConversion<uint64_t   , uint64_t   , 3, 4>>( offset, arraySize ) );
          case PT_UINT64 | PT_VECTOR4    : return( std::make_shared<ParameterInfoVectorConversion<uint64_t   , uint64_t   , 4, 4>>( offset, arraySize ) );
          case PT_BOOL                   : return( std::make_shared<ParameterInfoVectorConversion<bool       , uint32_t   , 1, 1>>( offset, arraySize ) );
          case PT_BOOL | PT_VECTOR2      : return( std::make_shared<ParameterInfoVectorConversion<bool       , uint32_t   , 2, 2>>( offset, arraySize ) );
          case PT_BOOL | PT_VECTOR3      : return( std::make_shared<ParameterInfoVectorConversion<bool       , uint32_t   , 3, 4>>( offset, arraySize ) );
          case PT_BOOL | PT_VECTOR4      : return( std::make_shared<ParameterInfoVectorConversion<bool       , uint32_t   , 4, 4>>( offset, arraySize ) );
          case PT_FLOAT32 | PT_MATRIX2x2 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float32, dp::Float32, 2, 2, 2>>( offset, arraySize ) );
          case PT_FLOAT32 | PT_MATRIX2x3 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float32, dp::Float32, 3, 4, 2>>( offset, arraySize ) );
          case PT_FLOAT32 | PT_MATRIX2x4 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float32, dp::Float32, 4, 4, 2>>( offset, arraySize ) );
          case PT_FLOAT32 | PT_MATRIX3x2 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float32, dp::Float32, 2, 2, 3>>( offset, arraySize ) );
          case PT_FLOAT32 | PT_MATRIX3x3 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float32, dp::Float32, 3, 4, 3>>( offset, arraySize ) );
          case PT_FLOAT32 | PT_MATRIX3x4 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float32, dp::Float32, 4, 4, 3>>( offset, arraySize ) );
          case PT_FLOAT32 | PT_MATRIX4x2 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float32, dp::Float32, 2, 2, 4>>( offset, arraySize ) );
          case PT_FLOAT32 | PT_MATRIX4x3 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float32, dp::Float32, 3, 4, 4>>( offset, arraySize ) );
          case PT_FLOAT32 | PT_MATRIX4x4 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float32, dp::Float32, 4, 4, 4>>( offset, arraySize ) );
          case PT_FLOAT64                : return( std::make_shared<ParameterInfoVectorConversion<dp::Float64, dp::Float64, 1, 1>>( offset, arraySize ) );
          case PT_FLOAT64 | PT_VECTOR2   : return( std::make_shared<ParameterInfoVectorConversion<dp::Float64, dp::Float64, 2, 2>>( offset, arraySize ) );
          case PT_FLOAT64 | PT_VECTOR3   : return( std::make_shared<ParameterInfoVectorConversion<dp::Float64, dp::Float64, 3, 4>>( offset, arraySize ) );
          case PT_FLOAT64 | PT_VECTOR4   : return( std::make_shared<ParameterInfoVectorConversion<dp::Float64, dp::Float64, 4, 4>>( offset, arraySize ) );
          case PT_FLOAT64 | PT_MATRIX2x2 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float64, dp::Float64, 2, 2, 2>>( offset, arraySize ) );
          case PT_FLOAT64 | PT_MATRIX2x3 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float64, dp::Float64, 3, 4, 2>>( offset, arraySize ) );
          case PT_FLOAT64 | PT_MATRIX2x4 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float64, dp::Float64, 4, 4, 2>>( offset, arraySize ) );
          case PT_FLOAT64 | PT_MATRIX3x2 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float64, dp::Float64, 2, 2, 3>>( offset, arraySize ) );
          case PT_FLOAT64 | PT_MATRIX3x3 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float64, dp::Float64, 3, 4, 3>>( offset, arraySize ) );
          case PT_FLOAT64 | PT_MATRIX3x4 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float64, dp::Float64, 4, 4, 3>>( offset, arraySize ) );
          case PT_FLOAT64 | PT_MATRIX4x2 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float64, dp::Float64, 2, 2, 4>>( offset, arraySize ) );
          case PT_FLOAT64 | PT_MATRIX4x3 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float64, dp::Float64, 3, 4, 4>>( offset, arraySize ) );
          case PT_FLOAT64 | PT_MATRIX4x4 : return( std::make_shared<ParameterInfoMatrixConversion<dp::Float64, dp::Float64, 4, 4, 4>>( offset, arraySize ) );
          case PT_ENUM                   : return( std::make_shared<ParameterInfoVectorConversion<dp::fx::EnumSpec::StorageType, int, 1, 1>>( offset, arraySize ) );
          default:
            DP_ASSERT( !"Unsupported parametertype" );
            return dp::fx::ParameterGroupLayout::ParameterInfoSharedPtr();
        }
      }

    } // namespace glsl
  } // namespace fx
} // namespace fx
