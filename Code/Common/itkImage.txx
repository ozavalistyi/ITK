/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkImage.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkImage_txx
#define _itkImage_txx
#include "itkImage.h"
#include "itkProcessObject.h"
#include "itkAffineTransform.h"

namespace itk
{

/**
 *
 */
template<class TPixel, unsigned int VImageDimension>
Image<TPixel, VImageDimension>
::Image()
{
  m_Buffer = PixelContainer::New();

  unsigned int i;
  for (i=0; i < VImageDimension; i++)
    {
    m_Spacing[i] = 1.0;
    m_Origin[i] = 0.0;
    }

  typedef AffineTransform< double,  VImageDimension > AffineTransformType;

  m_IndexToPhysicalTransform = AffineTransformType::New();
  m_PhysicalToIndexTransform = AffineTransformType::New();

}


/**
 *
 */
template<class TPixel, unsigned int VImageDimension>
Image<TPixel, VImageDimension>
::~Image()
{
}


//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension>
void 
Image<TPixel, VImageDimension>
::Allocate()
{
  unsigned long num;

  this->ComputeOffsetTable();
  num = this->GetOffsetTable()[VImageDimension];
  
  m_Buffer->Reserve(num);
}

template<class TPixel, unsigned int VImageDimension>
void 
Image<TPixel, VImageDimension>
::Initialize()
{
  //
  // We don't modify ourselves because the "ReleaseData" methods depend upon
  // no modification when initialized.
  //

  // Call the superclass which should initialize the BufferedRegion ivar.
  Superclass::Initialize();

  // Replace the handle to the buffer. This is the safest thing to do,
  // since the same container can be shared by multiple images (e.g.
  // Grafted outputs and in place filters).
  m_Buffer = PixelContainer::New();
}


template<class TPixel, unsigned int VImageDimension>
void 
Image<TPixel, VImageDimension>
::FillBuffer (const TPixel& value)
{
  const unsigned long numberOfPixels =
          this->GetBufferedRegion().GetNumberOfPixels();

  for(unsigned int i=0; i<numberOfPixels; i++) 
    {
    (*m_Buffer)[i] = value;
    }
}

template<class TPixel, unsigned int VImageDimension>
void 
Image<TPixel, VImageDimension>
::SetPixelContainer(PixelContainer *container)
{
  if (m_Buffer != container)
    {
    m_Buffer = container;
    this->Modified();
    }
}
    


//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension>
void 
Image<TPixel, VImageDimension>
::SetSpacing(const double spacing[ImageDimension] )
{
  unsigned int i; 
  for (i=0; i<VImageDimension; i++)
    {
    if ( spacing[i] != m_Spacing[i] )
      {
      break;
      }
    } 
  if ( i < VImageDimension ) 
    { 
    for (i=0; i<VImageDimension; i++)
      {
      m_Spacing[i] = spacing[i];
      }
    this->RebuildTransforms();
    }
}

//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension>
void 
Image<TPixel, VImageDimension>
::SetSpacing(const float spacing[ImageDimension] )
{
  unsigned int i; 
  for (i=0; i<VImageDimension; i++)
    {
    if ( (double)spacing[i] != m_Spacing[i] )
      {
      break;
      }
    } 
  if ( i < VImageDimension ) 
    { 
    for (i=0; i<VImageDimension; i++)
      {
      m_Spacing[i] = spacing[i];
      }
  this->RebuildTransforms();
    }
}



//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension>
const double *
Image<TPixel, VImageDimension>
::GetSpacing() const
{
  return m_Spacing;
}




//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension>
void 
Image<TPixel, VImageDimension>
::SetOrigin(const double origin[ImageDimension] )
{
  unsigned int i; 
  for (i=0; i<VImageDimension; i++)
    {
    if ( origin[i] != m_Origin[i] )
      {
      break;
      }
    } 
  if ( i < VImageDimension ) 
    { 
    for (i=0; i<VImageDimension; i++)
      {
      m_Origin[i] = origin[i];
      }
    this->RebuildTransforms();
    }
}


//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension>
void 
Image<TPixel, VImageDimension>
::SetOrigin(const float origin[ImageDimension] )
{
  unsigned int i; 
  for (i=0; i<VImageDimension; i++)
    {
    if ( (double)origin[i] != m_Origin[i] )
      {
      break;
      }
    } 
  if ( i < VImageDimension ) 
    { 
    for (i=0; i<VImageDimension; i++)
      {
      m_Origin[i] = origin[i];
      }
    this->RebuildTransforms();
    }
}

//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension>
const double *
Image<TPixel, VImageDimension>
::GetOrigin() const
{
  return m_Origin;
}



//---------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension>
void 
Image<TPixel, VImageDimension>
::RebuildTransforms() throw ( std::exception )
{
  typedef AffineTransform< double, VImageDimension > AffineTransformType;

  typename AffineTransformType::MatrixType matrix;
  typename AffineTransformType::OffsetType offset;

  for (unsigned int i = 0; i < VImageDimension; i++)
    {
    for (unsigned int j = 0; j < VImageDimension; j++)
      {
      matrix[i][j] = 0.0;
      }
    matrix[i][i] = m_Spacing[i];
    offset[i]    = m_Origin [i];
    }

  // Create a new transform if one doesn't already exist
  if ( !m_IndexToPhysicalTransform )
    {
    m_IndexToPhysicalTransform = 
                AffineTransformType::New().GetPointer();
    }

  // WARNING this will throw an exception if the 
  // actual Transform stored in the image is not an
  // Affine Transform !!
  typename AffineTransformType::Pointer affineTransform =
           dynamic_cast< AffineTransformType * >( 
               m_IndexToPhysicalTransform.GetPointer() );

  affineTransform->SetMatrix(matrix);
  affineTransform->SetOffset(offset);

  for (unsigned int i = 0; i < VImageDimension; i++)
    {
    for (unsigned int j = 0; j < VImageDimension; j++)
      {
      matrix[i][j] = 0.0;
      }
    matrix[i][i] = 1.0 / m_Spacing[i];
    offset[i]    = -m_Origin[i] / m_Spacing[i];
    }

  // Create a new transform if one doesn't already exist
  if( !m_PhysicalToIndexTransform  )
    {
    m_PhysicalToIndexTransform = 
                AffineTransformType::New().GetPointer();
    }
  

  affineTransform = dynamic_cast< AffineTransformType * >( 
                              m_PhysicalToIndexTransform.GetPointer() );

  affineTransform->SetMatrix(matrix);
  affineTransform->SetOffset(offset);

  this->Modified();

}


//---------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension>
typename Image<TPixel, VImageDimension>::TransformPointer
Image<TPixel, VImageDimension>
::GetIndexToPhysicalTransform(void) const
{     
  return m_IndexToPhysicalTransform.GetPointer();
}


//---------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension>
typename Image<TPixel, VImageDimension>::TransformPointer
Image<TPixel, VImageDimension>
::GetPhysicalToIndexTransform(void) const
{
  return m_PhysicalToIndexTransform.GetPointer();
}

//----------------------------------------------------------------------------
template<class TPixel, unsigned int VImageDimension>
void
Image<TPixel, VImageDimension>
::CopyInformation(const DataObject *data)
{
  // Standard call to the superclass' method
  Superclass::CopyInformation(data);

  // Attempt to cast data to an ImageBase.  All subclasses of ImageBase
  // respond to GetSpacing(), GetOrigin()
  const ImageBase<VImageDimension> *phyData;
  
  phyData = dynamic_cast<const ImageBase<VImageDimension>*>(data);

  if (phyData)
    {
    // Copy the origin and spacing
    this->SetSpacing( phyData->GetSpacing() );
    this->SetOrigin( phyData->GetOrigin() );
    }
  else
    {
    // pointer could not be cast back down
    itkExceptionMacro(<< "itk::Image::CopyInformation() cannot cast "
              << typeid(data).name() << " to "
              << typeid(ImageBase<VImageDimension>*).name() );
    }
}


/**
 *
 */
template<class TPixel, unsigned int VImageDimension>
void 
Image<TPixel, VImageDimension>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  
  unsigned int i;
  os << indent << "Origin: [";
  for (i=0; i+1 < VImageDimension; i++)
    {
    os << m_Origin[i] << ", ";
    }
  os << m_Origin[i] << "]" << std::endl;
  os << indent << "Spacing: [";
  for (i=0; i+1 < VImageDimension; i++)
    {
    os << m_Spacing[i] << ", ";
    }
  os << m_Spacing[i] << "]" << std::endl;

  os << indent << "PixelContainer: " << m_Buffer << std::endl;
  if (m_IndexToPhysicalTransform)
    {       
    os << indent << "IndexToPhysicalTransform: " << m_IndexToPhysicalTransform << std::endl;
    }
  if (m_PhysicalToIndexTransform)
    {
    os << indent << "PhysicalToIndexTransform: " << m_PhysicalToIndexTransform << std::endl;
    }
}


} // end namespace itk

#endif
