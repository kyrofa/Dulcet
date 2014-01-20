// Qt includes
#include <QDebug>
#include <QMimeDatabase>

// TagLib includes
#include <taglib/mpegfile.h>
#include <taglib/mp4file.h>
#include <taglib/flacfile.h>
#include <taglib/wavfile.h>
#include <taglib/oggfile.h>
#include <taglib/oggflacfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/asffile.h>
#include <taglib/aifffile.h>
#include <taglib/apefile.h>
#include <taglib/xmfile.h>
#include <taglib/trueaudiofile.h>
#include <taglib/modfile.h>
#include <taglib/s3mfile.h>
#include <taglib/itfile.h>

#include "music_file_type_resolver.h"

MusicFileTypeResolver::MusicFileTypeResolver() :
   QMimeDatabase(),
   TagLib::FileRef::FileTypeResolver()
{
}

MusicFileTypeResolver::~MusicFileTypeResolver()
{
}

TagLib::File* MusicFileTypeResolver::createFile(TagLib::FileName fileName,
                                                bool readAudioProperties,
                                                TagLib::AudioProperties::ReadStyle audioPropertiesStyle) const
{
	QMimeType mimeType = mimeTypeForFile(fileName);

	if (mimeType.inherits("audio/mpeg"))
	{
		return new TagLib::MPEG::File(fileName, readAudioProperties, audioPropertiesStyle);
	}

	if (mimeType.inherits("audio/mp4") || mimeType.inherits("video/mp4") || mimeType.inherits("video/mpeg"))
	{
		return new TagLib::MP4::File(fileName, readAudioProperties, audioPropertiesStyle);
	}

	if (mimeType.inherits("audio/flac"))
	{
		return new TagLib::FLAC::File(fileName, readAudioProperties, audioPropertiesStyle);
	}

	if (mimeType.inherits("audio/x-wav"))
	{
		return new TagLib::RIFF::WAV::File(fileName, readAudioProperties, audioPropertiesStyle);
	}

	if (mimeType.inherits("audio/ogg") || mimeType.inherits("video/ogg"))
	{
		// Not sure how Qt's mime detection is with different audio in the Ogg container...
		// let's try FLAC first, then Vorbis (like the default create() function in TagLib).
		TagLib::File *file = new TagLib::Ogg::FLAC::File(fileName, readAudioProperties, audioPropertiesStyle);
		if (file->isValid())
		{
			return file;
		}

		delete file;
		return new TagLib::Ogg::Vorbis::File(fileName, readAudioProperties, audioPropertiesStyle);
	}

	if (mimeType.inherits("audio/x-ms-wma") || mimeType.inherits("video/x-ms-asf"))
	{
		return new TagLib::ASF::File(fileName, readAudioProperties, audioPropertiesStyle);
	}

	if (mimeType.inherits("audio/x-aiff"))
	{
		return new TagLib::RIFF::AIFF::File(fileName, readAudioProperties, audioPropertiesStyle);
	}

	if (mimeType.inherits("audio/x-ape"))
	{
		return new TagLib::APE::File(fileName, readAudioProperties, audioPropertiesStyle);
	}

	if (mimeType.inherits("audio/x-tta"))
	{
		return new TagLib::TrueAudio::File(fileName, readAudioProperties, audioPropertiesStyle);
	}

	if (mimeType.inherits("audio/x-mod"))
	{
		return new TagLib::Mod::File(fileName, readAudioProperties, audioPropertiesStyle);
	}

	if (mimeType.inherits("audio/x-s3m"))
	{
		return new TagLib::S3M::File(fileName, readAudioProperties, audioPropertiesStyle);
	}

	if (mimeType.inherits("audio/x-it"))
	{
		return new TagLib::IT::File(fileName, readAudioProperties, audioPropertiesStyle);
	}

	if (mimeType.inherits("audio/x-xm"))
	{
		return new TagLib::XM::File(fileName, readAudioProperties, audioPropertiesStyle);
	}

	return NULL;
}
