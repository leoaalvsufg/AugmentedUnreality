/*
Copyright 2016 Krzysztof Lis

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "AugmentedUnreality.h"
#include "AURVideoSourceCamera.h"

UAURVideoSourceCamera::UAURVideoSourceCamera()
	: CameraIndex(0)
	, DesiredResolution(0, 0)
	, OfferedResolutions{FIntPoint(1920, 1080), FIntPoint(1280, 720), FIntPoint(640, 480), FIntPoint(480, 360)}
	, Autofocus(true)
{
}

FString UAURVideoSourceCamera::GetIdentifier() const
{
	return FString::Printf(TEXT("DesktopCamera_%d"), CameraIndex);
}

FText UAURVideoSourceCamera::GetSourceName() const
{
	return FText::Format(NSLOCTEXT("AUR", "VideoSourceDesktopCamera", "Camera {0}"), FText::AsNumber(CameraIndex));

	/*
	if(!SourceName.IsEmpty())
	{
		return SourceName;
	}
	else
	{
		return FText::FromString(FString::Printf(TEXT("Camera %d"), CameraIndex));
	}
	*/
}

void UAURVideoSourceCamera::DiscoverConfigurations()
{
	Configurations.Empty();

	//= cv::VideoCapture does not work on Android
#if PLATFORM_WINDOWS || PLATFORM_LINUX
	for (auto const& resolution : OfferedResolutions)
	{
		FAURVideoConfiguration cfg(this, ResolutionToString(resolution));
		cfg.Resolution = resolution;

		Configurations.Add(cfg);
	}
#endif
}

bool UAURVideoSourceCamera::Connect(FAURVideoConfiguration const& configuration)
{
#if !PLATFORM_ANDROID
	try
	{
#endif
		Capture.open(CameraIndex);

		if (Capture.isOpened())
		{
			UE_LOG(LogAUR, Log, TEXT("UAURVideoSourceCamera::Connect: Connected to Camera %d"), CameraIndex)

#if !PLAFROM_LINUX
			// Suggest resolution
			if(configuration.Resolution.GetMin() > 0)
			{
				Capture.set(cv::CAP_PROP_FRAME_WIDTH, configuration.Resolution.X);
				Capture.set(cv::CAP_PROP_FRAME_HEIGHT, configuration.Resolution.Y);
			}

			// Suggest autofocus
			Capture.set(cv::CAP_PROP_AUTOFOCUS, Autofocus ? 1 : 0);
#endif

			LoadCalibration();
		}
		else
		{
			UE_LOG(LogAUR, Error, TEXT("UAURVideoSourceCamera::Connect: Failed to open Camera %d"), CameraIndex)
		}
#if !PLATFORM_ANDROID
	}
	catch (std::exception& exc)
	{
		UE_LOG(LogAUR, Error, TEXT("Exception in UAURVideoSourceCamera::Connect:\n	%s\n"), UTF8_TO_TCHAR(exc.what()))
		return false;
	}
#endif
	
	return Capture.isOpened();
}
