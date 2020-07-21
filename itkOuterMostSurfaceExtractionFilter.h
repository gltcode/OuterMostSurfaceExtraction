/*====================================================
* Copyright 2019 Ariemedi LLC.
* License: BSD
* Author: Wenjie Li, Longteng Guo
* Date: 2019.05.23
====================================================*/
#ifndef itkOuterMostSurfaceExtractionFilter_h
#define itkOuterMostSurfaceExtractionFilter_h

#include <vector>

#include "itkImageToImageFilter.h"
#include "itkMacro.h"
#include "itkImage.h"
#include "itkBarrier.h"
#include <itkExtractImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkLabelShapeKeepNObjectsImageFilter.h>
#include <itkImageRegionSplitterBase.h>
#include "itkImageRegionSplitterInSpecifiedDirection.h"
#include "itkSimpleFastMutexLock.h"
#include "itkMutexLockHolder.h"
#include <itkImageRegionIterator.h>

// itk threshold filters
#include <itkLiThresholdImageFilter.h>
#include <itkHuangThresholdImageFilter.h>
#include <itkIntermodesThresholdImageFilter.h>
#include <itkIsoDataThresholdImageFilter.h>
#include <itkKittlerIllingworthThresholdImageFilter.h>
#include <itkMaximumEntropyThresholdImageFilter.h>
#include <itkMomentsThresholdImageFilter.h>
#include <itkOtsuThresholdImageFilter.h>
#include <itkRenyiEntropyThresholdImageFilter.h>
#include <itkShanbhagThresholdImageFilter.h>
#include <itkTriangleThresholdImageFilter.h>
#include <itkYenThresholdImageFilter.h>

namespace itk
{
	/** Class description
	*
	* \class OuterMostSurfaceExtractionFilter
	*
	* \brife Get the outermost surface of an image.
	*
	* This filter...
	*/

	template<typename TImage>
	class OuterMostSurfaceExtractionFilter :public ImageToImageFilter<TImage, TImage>
	{
	public:
		enum ThresholdMethod
		{
			Huang = 0,
			Intermodes,
			IsoData,
			KittlerIllingworth,
			Li,
			MaximumEntropy,
			Moments,
			Otsu,
			RenyiEntropy,
			Shanbhag,
			Triangle,
			Yen
		};

		enum PlaneDirection
		{
			Two = 2,
			One = 1,
			Zero = 0,
			None = -1,
		};

		enum ImageModality
		{
			Undefined = -1,
			CT = 0,
			MRI = 1,
		};

		/** Standard ITK typedefs. */
		typedef OuterMostSurfaceExtractionFilter                    Self;
		typedef ImageToImageFilter<TImage, TImage>                  Superclass;
		typedef SmartPointer< Self >                                Pointer;
		typedef SmartPointer< const Self >                          ConstPointer;

		/** Alias for the image's type */
		typedef TImage           ImageType;
		typedef typename TImage::RegionType RegionType;
		/** Run-time type information (and related methods). */
		itkTypeMacro(OuterMostSurfaceExtractionFilter, ImageToImageFilter);

		/** Method for creation through the object factory. */
		itkNewMacro(Self);



		/** Set the name of the file containing the parameters. */
		itkSetMacro(ThresholdMethod, ThresholdMethod);
		itkSetMacro(ProcessDirectionList, std::vector<PlaneDirection>);
		itkSetMacro(ImageModality, ImageModality);

	protected:
		OuterMostSurfaceExtractionFilter();
		~OuterMostSurfaceExtractionFilter();

		void BeforeThreadedGenerateData() ITK_OVERRIDE;

		void AfterThreadedGenerateData() ITK_OVERRIDE;

		void ThreadedGenerateData(const RegionType & outputRegionForThread, ThreadIdType threadId) ITK_OVERRIDE;

		void EnlargeOutputRequestedRegion(DataObject * itkNotUsed(output)) ITK_OVERRIDE;
	private:
		ThresholdMethod m_ThresholdMethod;
		std::vector<PlaneDirection> m_ProcessDirectionList;
		ImageModality m_ImageModality;
		int ProgressOfProcess;

		const ImageRegionSplitterBase* GetImageRegionSplitter() const ITK_OVERRIDE;

		SimpleFastMutexLock SplitterLock;
		ImageRegionSplitterInSpecifiedDirection::Pointer Splitter;

		void Wait()
		{
				m_Barrier->Wait();	
		}
		typename Barrier::Pointer m_Barrier;

		typename ImageType::Pointer m_TemporaryImageData;

	};
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOuterMostSurfaceExtractionFilter.hxx"
#endif

#endif // !itkOuterMostSurfaceExtractionFilter_h


