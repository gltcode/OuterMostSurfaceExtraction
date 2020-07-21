/*====================================================
* Copyright 2019 Ariemedi LLC.
* License: BSD
* Author: Wenjie Li, Longteng Guo
* Date: 2019.05.23
====================================================*/
#include "itkOuterMostSurfaceExtractionFilter.h"
#include <Windows.h>


namespace itk
{
	template< typename TImage >
	OuterMostSurfaceExtractionFilter< TImage >
		::OuterMostSurfaceExtractionFilter()
	{
		std::cout << "Construct function" << std::endl;
		m_ThresholdMethod = ThresholdMethod::Otsu;

		m_ProcessDirectionList.push_back(OuterMostSurfaceExtractionFilter::None);
		m_ProcessDirectionList.push_back(OuterMostSurfaceExtractionFilter::None);
		m_ImageModality = OuterMostSurfaceExtractionFilter::Undefined;
		ProgressOfProcess = 0;
		Splitter = ImageRegionSplitterInSpecifiedDirection::New().GetPointer();
		// thread safe lazy initialization, prevent race condition on
		// setting, with an atomic set if null.
		MutexLockHolder< SimpleFastMutexLock > lock(SplitterLock);
	}

	template< typename TImage >
	OuterMostSurfaceExtractionFilter< TImage >
		::~OuterMostSurfaceExtractionFilter()
	{
		std::cout << "Deconstruct function" << std::endl;
	}
	template< typename TImage >
	void OuterMostSurfaceExtractionFilter< TImage >
		::EnlargeOutputRequestedRegion(DataObject *)
	{
		this->GetOutput()
			->SetRequestedRegion(this->GetOutput()->GetLargestPossibleRegion());
	}
	template< typename TImage >
	void OuterMostSurfaceExtractionFilter< TImage >
		::BeforeThreadedGenerateData()
	{
		typedef itk::ImageRegionIterator<ImageType> It3DType;
		short EdgeValue;
		int handlevale = 1;

		if (ProgressOfProcess == 0)
		{
			std::cout << "First process start." << std::endl;
			m_TemporaryImageData = ImageType::New();
			m_TemporaryImageData->Graft(const_cast<ImageType *>(this->GetInput()));

			if (m_ProcessDirectionList.size() != 2 || m_ProcessDirectionList.at(0) == OuterMostSurfaceExtractionFilter::None || m_ProcessDirectionList.at(1) == OuterMostSurfaceExtractionFilter::None)
			{
				this->GraftOutput(m_TemporaryImageData);
				itkExceptionMacro("The filter need only two dimision-numbers.");
				itkExceptionMacro("Please Use The Method SetProcessDirectionList() Set the Axial dimision number and Sagittal dimision number.");
				return;
			}
			if (m_ImageModality == OuterMostSurfaceExtractionFilter::Undefined)
			{
				this->GraftOutput(m_TemporaryImageData);
				itkExceptionMacro("Please Use The Method SetImageModality() Specify The Modality Of The Image!");
				return;
			}
			// Pre process

			typename ImageType::RegionType inputReigon = m_TemporaryImageData->GetLargestPossibleRegion();
			typename ImageType::IndexType start = inputReigon.GetIndex();
			typename ImageType::SizeType size = inputReigon.GetSize();
			It3DType it(m_TemporaryImageData, inputReigon);


			if (m_ImageModality == OuterMostSurfaceExtractionFilter::CT)
			{
				it.GoToBegin();
				while (!it.IsAtEnd())
				{

					typename ImageType::PixelType Value = it.Get();
					if (Value < -450)
						it.Set(-450);
					if (Value > -150)
						it.Set(-150);
					++it;
				}
			}
			if (m_ImageModality == OuterMostSurfaceExtractionFilter::MRI)
			{
				it.GoToBegin();
				short MaxValue = 0;
				while (!it.IsAtEnd())
				{
					typename ImageType::PixelType Value = it.Get();
					if (Value > MaxValue)
					{
						MaxValue = Value;
					}
					++it;
				}
				std::vector<long> relativeFrequency;
				relativeFrequency.resize(MaxValue + 1);
				std::vector<long> relativeSumFrequency;
				relativeSumFrequency.resize(MaxValue + 1);
				for (int j = 0; j < MaxValue + 1; j++)
				{
					relativeFrequency[j] = 0.0;
					relativeSumFrequency[j] = 0.0;
				}

				it.GoToBegin();
				while (!it.IsAtEnd())
				{
					typename ImageType::PixelType Value = it.Get();
					relativeFrequency.at(Value)++;
					++it;
				}
				std::vector<long>::iterator iter1 = relativeFrequency.begin();
				std::vector<long>::iterator iter2 = relativeSumFrequency.begin();

				while (iter1 != relativeFrequency.end())
				{
					if (iter1 == relativeFrequency.begin())
					{
						*iter2 = *iter1;
					}
					else
					{
						*iter2 = *(iter2 - 1) + *iter1;
					}
					//	OutPutHistogram << *iter1 << endl;

					++iter1;
					++iter2;
				}


				float Voxelnumber = 0.9996*inputReigon.GetNumberOfPixels();
				//cout << "Voxelnumber is " << Voxelnumber << endl;
				short ThresholdPixelValue;
				iter2 = relativeSumFrequency.begin();
				short Slice = 0;
				while (iter2 != relativeSumFrequency.end())
				{
					if (*iter2 > Voxelnumber)
					{
						ThresholdPixelValue = Slice;
						break;
					}
					iter2++;
					Slice++;

				}
				short MaxDisplayValue = 2560;
				it.GoToBegin();
				while (!it.IsAtEnd())
				{
					typename ImageType::PixelType PixelValue = it.Get();
					if (PixelValue > ThresholdPixelValue)
					{
						it.Set(MaxDisplayValue);
					}
					else
					{
						it.Set((short)(PixelValue * MaxDisplayValue / ThresholdPixelValue));  //线性映射
					}
					//it->SetPixel(IndexSliceImage, (short)(P(PixelValue)*MaxValue / (SliceSize[0] * SliceSize[1])));//直方图均衡化
					++it;
				}
				it.GoToBegin();
				while (!it.IsAtEnd())
				{
					typename ImageType::PixelType Value = it.Get();
					if (Value < 200)
						it.Set(0);
					++it;
				}
			}


			if (m_ImageModality == OuterMostSurfaceExtractionFilter::CT)
			{
				EdgeValue = -450;
			}
			if (m_ImageModality == OuterMostSurfaceExtractionFilter::MRI)
			{
				EdgeValue = 0;
			}

			for (int x = 0; x < size[0]; x++)
			{
				if (x < 0 + handlevale || x > size[0] - 1 - handlevale)
				{
					for (int y = 0; y < size[1]; y++)
						for (int z = 0; z < size[2]; z++)
						{
							start[0] = x;
							start[1] = y;
							start[2] = z;
							m_TemporaryImageData->SetPixel(start, EdgeValue);
						}
				}
				else
				{
					for (int y = 0; y < size[1]; y++)
						for (int z = 0; z < size[2]; z++)
							if (y <0 + handlevale || y >size[1] - 1 - handlevale || z <0 + handlevale || z >size[2] - 1 - handlevale)
							{
								start[0] = x;
								start[1] = y;
								start[2] = z;
								m_TemporaryImageData->SetPixel(start, EdgeValue);
							}
				}

			}
		}
		else if (ProgressOfProcess==1)
		{
			std::cout << "Second process start." << std::endl;

			typename ImageType::RegionType inputReigon = m_TemporaryImageData->GetLargestPossibleRegion();
			typename ImageType::IndexType start = inputReigon.GetIndex();
			typename ImageType::SizeType size = inputReigon.GetSize();
			EdgeValue = 1000;
			for (int x = 0; x < size[0]; x++)
			{
				if (x < 0 + handlevale || x > size[0] - 1 - handlevale)
				{
					for (int y = 0; y < size[1]; y++)
						for (int z = 0; z < size[2]; z++)
						{
							start[0] = x;
							start[1] = y;
							start[2] = z;
							m_TemporaryImageData->SetPixel(start, EdgeValue);
						}
				}
				else
				{
					for (int y = 0; y < size[1]; y++)
						for (int z = 0; z < size[2]; z++)
							if (y <0 + handlevale || y >size[1] - 1 - handlevale || z <0 + handlevale || z >size[2] - 1 - handlevale)
							{
								start[0] = x;
								start[1] = y;
								start[2] = z;
								m_TemporaryImageData->SetPixel(start, EdgeValue);
							}
				}

			}
		}

		ThreadIdType nbOfThreads = this->GetNumberOfThreads();
		if (itk::MultiThreader::GetGlobalMaximumNumberOfThreads() != 0)
		{
			nbOfThreads = std::min(this->GetNumberOfThreads(), itk::MultiThreader::GetGlobalMaximumNumberOfThreads());
		}
		// number of threads can be constrained by the region size, so call the
		// SplitRequestedRegion
		// to get the real number of threads which will be used
		typename TImage::RegionType splitRegion;  // dummy region - just to call
														// the following method
		nbOfThreads = this->SplitRequestedRegion(0, nbOfThreads, splitRegion);

		// set up the vars used in the threads
		m_Barrier = Barrier::New();
		m_Barrier->Initialize(nbOfThreads);

	}
	template< typename TImage >
	void OuterMostSurfaceExtractionFilter< TImage >
		::ThreadedGenerateData(const RegionType & outputRegionForThread,
			ThreadIdType threadId)
	{
		// Typedefs
		typedef short PixelType;
		typedef itk::Image<PixelType, 2> SliceImageType;
		typedef itk::ExtractImageFilter<ImageType, SliceImageType> FilterType;
		typedef itk::ConnectedComponentImageFilter <SliceImageType, SliceImageType > ConnectedComponentImageFilterType;

		typedef itk::ImageRegionIterator<ImageType> It3DType;
		typedef itk::ImageRegionIterator<SliceImageType> It2DType;

		// Threshold filters
		typedef itk::LiThresholdImageFilter<SliceImageType, SliceImageType > LiFilterType;
		typedef itk::HuangThresholdImageFilter<SliceImageType, SliceImageType > HuangFilterType;
		typedef itk::IntermodesThresholdImageFilter<SliceImageType, SliceImageType > IntermodesFilterType;
		typedef itk::IsoDataThresholdImageFilter<SliceImageType, SliceImageType > IsoDataFilterType;
		typedef itk::KittlerIllingworthThresholdImageFilter<SliceImageType, SliceImageType > KittlerIllingworthFilterType;
		typedef itk::LiThresholdImageFilter<SliceImageType, SliceImageType > LiFilterType;
		typedef itk::MaximumEntropyThresholdImageFilter<SliceImageType, SliceImageType > MaximumEntropyFilterType;
		typedef itk::MomentsThresholdImageFilter<SliceImageType, SliceImageType > MomentsFilterType;
		typedef itk::OtsuThresholdImageFilter<SliceImageType, SliceImageType > OtsuFilterType;
		typedef itk::RenyiEntropyThresholdImageFilter<SliceImageType, SliceImageType >	RenyiEntropyFilterType;
		typedef itk::ShanbhagThresholdImageFilter<SliceImageType, SliceImageType >	ShanbhagFilterType;
		typedef itk::TriangleThresholdImageFilter<SliceImageType, SliceImageType >	TriangleFilterType;
		typedef itk::YenThresholdImageFilter<SliceImageType, SliceImageType > YenFilterType;

		// Process pipeline

		// Construct threshold filters container
		typedef std::map<OuterMostSurfaceExtractionFilter::ThresholdMethod, itk::HistogramThresholdImageFilter<SliceImageType, SliceImageType>::Pointer> FilterContainerType;
		FilterContainerType filterContainer;
		filterContainer[OuterMostSurfaceExtractionFilter::Huang] = HuangFilterType::New();
		filterContainer[OuterMostSurfaceExtractionFilter::Intermodes] = IntermodesFilterType::New();
		filterContainer[OuterMostSurfaceExtractionFilter::IsoData] = IsoDataFilterType::New();
		filterContainer[OuterMostSurfaceExtractionFilter::KittlerIllingworth] = KittlerIllingworthFilterType::New();
		filterContainer[OuterMostSurfaceExtractionFilter::Li] = LiFilterType::New();
		filterContainer[OuterMostSurfaceExtractionFilter::MaximumEntropy] = MaximumEntropyFilterType::New();
		filterContainer[OuterMostSurfaceExtractionFilter::Moments] = MomentsFilterType::New();
		filterContainer[OuterMostSurfaceExtractionFilter::Otsu] = OtsuFilterType::New();
		filterContainer[OuterMostSurfaceExtractionFilter::RenyiEntropy] = RenyiEntropyFilterType::New();
		filterContainer[OuterMostSurfaceExtractionFilter::Shanbhag] = ShanbhagFilterType::New();
		filterContainer[OuterMostSurfaceExtractionFilter::Triangle] = TriangleFilterType::New();
		filterContainer[OuterMostSurfaceExtractionFilter::Yen] = YenFilterType::New();

		// Find recommended threshold filter
		FilterContainerType::iterator it = filterContainer.find(m_ThresholdMethod);
		if (it == filterContainer.end())
		{
			std::cout << "Threshold filter not found!" << std::endl;
			return;
		}
		PlaneDirection currentDirection = m_ProcessDirectionList.at(ProgressOfProcess);
		
		typename ImageType::SizeType size = outputRegionForThread.GetSize();
		size = outputRegionForThread.GetSize();
		unsigned int TotalSliceNumber = size[currentDirection];
		size[currentDirection] = 0;
		typename ImageType::IndexType start = outputRegionForThread.GetIndex();

		//遍历某个维度（x\y\z维度）上的每一张图像
		for (int slicenumber =start[currentDirection]; slicenumber < start[currentDirection]+TotalSliceNumber; slicenumber++)
		{
			typename ImageType::IndexType start = outputRegionForThread.GetIndex();
			start[currentDirection] = slicenumber;

			typename ImageType::RegionType desiredRegion;
			desiredRegion.SetSize(size);
			desiredRegion.SetIndex(start);

			FilterType::Pointer ExtractSlicefilter = FilterType::New();
			ExtractSlicefilter->InPlaceOn();
			ExtractSlicefilter->SetDirectionCollapseToSubmatrix();
			ExtractSlicefilter->SetExtractionRegion(desiredRegion);
			ExtractSlicefilter->SetInput(m_TemporaryImageData);
			ExtractSlicefilter->Update();

			typename SliceImageType::Pointer image = SliceImageType::New();
			image = ExtractSlicefilter->GetOutput();

			if (ProgressOfProcess == 0)
			{
				(*it).second->SetInsideValue(255);
				(*it).second->SetOutsideValue(0);
				(*it).second->SetNumberOfHistogramBins(25);
				(*it).second->SetInput(image);
				//(*it).second->Update();

				try
				{
					(*it).second->Update();
				}
				catch (itk::ExceptionObject & err)
				{
					std::cout << "Caught exception" << std::endl;
					std::cout << err << std::endl;
					this->GraftOutput(m_TemporaryImageData);
					return;
				}
				image = (*it).second->GetOutput();

			}

			ConnectedComponentImageFilterType::Pointer connected = ConnectedComponentImageFilterType::New();
			connected->SetInput(image);
			//#########################################################
			typedef itk::LabelShapeKeepNObjectsImageFilter< SliceImageType > LabelShapeKeepNObjectsImageFilterType;
			LabelShapeKeepNObjectsImageFilterType::Pointer labelShapeKeepNObjectsImageFilter = LabelShapeKeepNObjectsImageFilterType::New();
			labelShapeKeepNObjectsImageFilter->SetInput(connected->GetOutput());
			labelShapeKeepNObjectsImageFilter->SetBackgroundValue(0);
			labelShapeKeepNObjectsImageFilter->SetNumberOfObjects(1);
			labelShapeKeepNObjectsImageFilter->SetAttribute(LabelShapeKeepNObjectsImageFilterType::LabelObjectType::NUMBER_OF_PIXELS);
			labelShapeKeepNObjectsImageFilter->Update();
			//#########################################################

			image = labelShapeKeepNObjectsImageFilter->GetOutput();

			typename SliceImageType::RegionType TwoDRegion = image->GetLargestPossibleRegion();
			It2DType it2D(image, TwoDRegion);
			it2D.GoToBegin();
			while (!it2D.IsAtEnd())
			{
				typename SliceImageType::PixelType value = it2D.Get();
				if (value != 0 && value != 1)
				{
					it2D.Set(1);
				}
				++it2D;
			}

			// Label image to normal itk image
			typename SliceImageType::IndexType index2d;
			typename ImageType::IndexType index3d;
			typename SliceImageType::SizeType TwoDSize = TwoDRegion.GetSize();
			if (ProgressOfProcess == 0)
			{
				for (int x = 0; x < TwoDSize[0]; x++)
					for (int y = 0; y < TwoDSize[1]; y++)
					{
						index2d[0] = x;
						index2d[1] = y;

						if (currentDirection == 2)
						{
							index3d[0] = x;
							index3d[1] = y;
							index3d[2] = slicenumber;
						}
						if (currentDirection == 1)
						{
							index3d[0] = x;
							index3d[1] = slicenumber;
							index3d[2] = y;
						}
						if (currentDirection == 0)
						{
							index3d[0] = slicenumber;
							index3d[1] = x;
							index3d[2] = y;
						}
						short value = 0;
						value = image->GetPixel(index2d);
						if (value == 1)
							value = 1000;
						else
							value = 0;
						m_TemporaryImageData->SetPixel(index3d, value);

					}
			}

			if (ProgressOfProcess == 1)
			{
				for (int x = 0; x < TwoDSize[0]; x++)
					for (int y = 0; y < TwoDSize[1]; y++)
					{
						index2d[0] = x;
						index2d[1] = y;
						if (currentDirection == 2)
						{
							index3d[0] = x;
							index3d[1] = y;
							index3d[2] = slicenumber;
						}
						if (currentDirection == 1)
						{
							index3d[0] = x;
							index3d[1] = slicenumber;
							index3d[2] = y;
						}
						if (currentDirection == 0)
						{
							index3d[0] = slicenumber;
							index3d[1] = x;
							index3d[2] = y;
						}
						short value = 0;
						value = image->GetPixel(index2d);
						if (value == 1)
							value = -1000;
						else
							value = 1000;
						m_TemporaryImageData->SetPixel(index3d, value);

					}
			}
		}

		Wait();
	}
	template< typename TImage >
	void OuterMostSurfaceExtractionFilter< TImage >
		::AfterThreadedGenerateData()
	{
		ProgressOfProcess++;
		if (ProgressOfProcess < m_ProcessDirectionList.size())
		{// Call a method that can be overridden by a subclass to perform
		// some calculations prior to splitting the main computations into
		// separate threads
			this->BeforeThreadedGenerateData();

			// Set up the multithreaded processing
			ThreadStruct str;
			str.Filter = this;

			// Get the output pointer
			const OutputImageType *outputPtr = this->GetOutput();
			const ImageRegionSplitterBase * splitter = this->GetImageRegionSplitter();
			const unsigned int validThreads = splitter->GetNumberOfSplits(outputPtr->GetRequestedRegion(), this->GetNumberOfThreads());

			this->GetMultiThreader()->SetNumberOfThreads(validThreads);
			this->GetMultiThreader()->SetSingleMethod(this->ThreaderCallback, &str);

			// multithread the execution
			this->GetMultiThreader()->SingleMethodExecute();

			// Call a method that can be overridden by a subclass to perform
			// some calculations after all the threads have completed
			this->AfterThreadedGenerateData();
		}
		else
		{
			this->GraftOutput(m_TemporaryImageData);

		}
	}
	template< typename TImage >
	const ImageRegionSplitterBase*
		OuterMostSurfaceExtractionFilter< TImage >
		::GetImageRegionSplitter(void) const
	{
		if (m_ProcessDirectionList.at(ProgressOfProcess) == 0)
		{
			Splitter->SetSplitDirction(ImageRegionSplitterInSpecifiedDirection::PlaneDirection::Zero);
		}
		else if (m_ProcessDirectionList.at(ProgressOfProcess) == 1)
		{
			Splitter->SetSplitDirction(ImageRegionSplitterInSpecifiedDirection::PlaneDirection::One);
		}
		else if (m_ProcessDirectionList.at(ProgressOfProcess) == 2)
		{
			Splitter->SetSplitDirction(ImageRegionSplitterInSpecifiedDirection::PlaneDirection::Two);
		}
		return Splitter;
	}

}
