#pragma once


class Film {
public:


private:
	
	Localized<std::string> name;

	Date worldReleaseDate;
	Localized<Date> localReleaseDate;

	std::vector<Person::Id> staff;

	std::vector<Video::Id> videos;
	std::vector<Audio::Id> audios;
	std::vector<Subtitle::Id> subtitles;

};



