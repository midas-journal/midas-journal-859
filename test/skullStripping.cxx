/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

/* skullStripping.cxx
This is a demo program for testing the itkStripTsImageFilter for automatic skull-stripping.

version: 1.0
date: 20.04.2012
initial release

Stefan Bauer
Medical Image Analysis Group, Institute for Surgical Technology and Biomechanics, University of Bern
stefan.bauer@istb.unibe.ch
*/

#include "time.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkMaskImageFilter.h"

#include "itkStripTsImageFilter.h"


int main( int argc, char* argv[] )
{
  if( argc < 4 )
  {
    std::cerr << std::endl << argv[0] << "   patientImageFile atlasImageFile atlasMaskFile" << std::endl;
    return EXIT_FAILURE;
  }

  double startTime = time(0);


  std::string patientImageFilename = argv[1];
  std::string atlasImageFilename = argv[2];
  std::string atlasMaskFilename = argv[3];

  typedef itk::Image<int, 3> ImageType;
  typedef itk::Image<short, 3> AtlasImageType;
  typedef itk::Image<unsigned char, 3> AtlasLabelType;


  // image reading
  typedef itk::ImageFileReader<ImageType> ReaderType;
  typedef itk::ImageFileReader<AtlasImageType> AtlasReaderType;
  typedef itk::ImageFileReader<AtlasLabelType> LabelReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  AtlasReaderType::Pointer atlasReader = AtlasReaderType::New();
  LabelReaderType::Pointer labelReader = LabelReaderType::New();

  reader->SetFileName( patientImageFilename );
  atlasReader->SetFileName( atlasImageFilename );
  labelReader->SetFileName( atlasMaskFilename );

  try
    {
    reader->Update();
    atlasReader->Update();
    labelReader->Update();
    }
  catch ( itk::ExceptionObject &exception )
    {
    std::cerr << "Exception caught ! " << std::endl;
    std::cerr << &exception << std::endl;

    return EXIT_FAILURE;
    }


  // perform skull-stripping using stripTsImageFilter
  std::cout << std::endl << "Performing skull-stripping" << std::endl;

  // set up skull-stripping filter
  typedef itk::StripTsImageFilter<ImageType, AtlasImageType, AtlasLabelType> StripTsFilterType;
  StripTsFilterType::Pointer stripTsFilter = StripTsFilterType::New();

  // set the required inputs for the stripTsImageFilter
  stripTsFilter->SetInput( reader->GetOutput() );
  stripTsFilter->SetAtlasImage( atlasReader->GetOutput() );
  stripTsFilter->SetAtlasBrainMask( labelReader->GetOutput() );

  try
    {
    stripTsFilter->Update();
    }
  catch ( itk::ExceptionObject &exception )
    {
    std::cerr << "Exception caught ! " << std::endl;
    std::cerr << &exception << std::endl;

    return EXIT_FAILURE;
    }


  // mask the patient image using the output generated from the stripTsImageFilter as mask
  typedef itk::MaskImageFilter<ImageType, AtlasLabelType, ImageType> MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();

  maskFilter->SetInput1( reader->GetOutput() );
  maskFilter->SetInput2( stripTsFilter->GetOutput() );

  try
    {
    maskFilter->Update();
    }
  catch ( itk::ExceptionObject &exception )
    {
    std::cerr << "Exception caught ! " << std::endl;
    std::cerr << &exception << std::endl;

    return EXIT_FAILURE;
    }


  // write mask and masked patient image
  typedef itk::ImageFileWriter<AtlasLabelType> MaskWriterType;
  MaskWriterType::Pointer maskWriter = MaskWriterType::New();
  maskWriter->SetInput( stripTsFilter->GetOutput() );
  maskWriter->SetFileName( "outputMask.mha" );
  maskWriter->UseCompressionOn();

  typedef itk::ImageFileWriter<ImageType> ImageWriterType;
  ImageWriterType::Pointer imageWriter = ImageWriterType::New();
  imageWriter->SetInput( maskFilter->GetOutput() );
  imageWriter->SetFileName( "outputMaskedImage.mha" );
  imageWriter->UseCompressionOn();

  try
    {
    maskWriter->Update();
    imageWriter->Update();
    }
  catch ( itk::ExceptionObject &exception )
    {
    std::cerr << "Exception caught ! " << std::endl;
    std::cerr << &exception << std::endl;

    return EXIT_FAILURE;
    }


  double endTime = time(0);
  std::cout << "Total computation time: " << endTime-startTime << "seconds" << std::endl;


  return EXIT_SUCCESS;
}
