#ifndef MUSIC_FILE_TYPE_RESOLVER_H
#define MUSIC_FILE_TYPE_RESOLVER_H

// Qt includes
#include <QMimeDatabase>

// TagLib includes
#include <taglib/fileref.h>

class MusicFileTypeResolver : public QMimeDatabase, public TagLib::FileRef::FileTypeResolver
{
	public:
		MusicFileTypeResolver();
		~MusicFileTypeResolver();

		virtual TagLib::File* createFile(TagLib::FileName fileName,
		                                 bool readAudioProperties = true,
		                                 TagLib::AudioProperties::ReadStyle audioPropertiesStyle = TagLib::AudioProperties::Average) const;
};

#endif // MUSIC_FILE_TYPE_RESOLVER_H
