#include <renderwure/engine/GTAData.hpp>
#include <renderwure/engine/GTAEngine.hpp>
#include <renderwure/loaders/LoaderIPL.hpp>
#include <renderwure/loaders/LoaderDFF.hpp>
#include <renderwure/loaders/LoaderIDE.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <dirent.h>
#include <sys/types.h>

// Hacky functtion to get around the fact that none of the files are the right case
std::string findFileRealCase(const std::string& lowerPath) {
#ifdef __unix__
	size_t lastSlash = lowerPath.find_last_of("/");
	std::string fileDir = ".";
	if(lastSlash != lowerPath.npos) {
		fileDir = lowerPath.substr(0, lastSlash);
	}
	else {
		lastSlash = 0;
	}
	
	DIR* dp = opendir(fileDir.c_str());
	dirent* ep;
	if( dp != NULL) {
		while( ep = readdir(dp)) {
			std::string realName(ep->d_name);
			std::string lowerRealName = realName;
			for( size_t t = 0; t < lowerRealName.size(); ++t) {
				lowerRealName[t] = tolower(lowerRealName[t]);
			}
			if(lowerRealName == lowerPath.substr(lastSlash+1)) {
				return fileDir + "/" + realName;
			}
		}
		
		closedir(dp);
	}
	
	return lowerPath;
	
#else 
	// We'll just have to assume this means Windows for now.
	return lowerPath;
#endif
}

// Yet another hack function to fix these paths
std::string fixPath(std::string path) {
	for( size_t t = 0; t < path.size(); ++t) {
		path[t] = tolower(path[t]);
		if(path[t] == '\\') {
			path[t] = '/';
		}
	}
	return path;
}


GTAData::GTAData(const std::string& path)
: datpath(path), engine(nullptr)
{

}

void GTAData::load()
{
	parseDAT(datpath+"/data/default.dat");
	parseDAT(datpath+"/data/gta3.dat");
	
	fileLocations.insert({"wheels.DFF", {false, datpath+"/models/Generic/wheels.DFF"}});
	fileLocations.insert({"particle.txd", {false, datpath+"/models/particle.txd"}});
	loadDFF("wheels.DFF");
	loadTXD("particle.txd");
	
	loadCarcols(datpath+"/data/carcols.dat");
	loadWeather(datpath+"/data/timecyc.dat");
	loadWaterpro(datpath+"/data/waterpro.dat");
	loadWater(datpath+"/data/water.dat");
}

void GTAData::parseDAT(const std::string& path)
{
	std::ifstream datfile(path.c_str());
	
	if(!datfile.is_open()) 
	{
		std::cerr << "Failed to open gta.dat" << std::endl;
	}
	else
	{
		for(std::string line, cmd; std::getline(datfile, line);)
		{
			if(line.size() == 0 || line[0] == '#') continue;
			line.erase(line.size()-1);
			
			size_t space = line.find_first_of(' ');
			if(space != line.npos)
			{
				cmd = line.substr(0, space);
				if(cmd == "IDE")
				{
					loadIDE(line.substr(space+1));
				}
				else if(cmd == "SPLASH")
				{
					splash = line.substr(space+1);
				}
				else if(cmd == "COLFILE")
				{
					int zone  = atoi(line.substr(space+1,1).c_str());
					std::string file = line.substr(space+3);
					loadCOL(zone, file);
				}
				else if(cmd == "IPL")
				{
					std::string fixedpath = fixPath(line.substr(space+1));
					fixedpath = findFileRealCase(datpath + "/" + fixedpath);
					loadIPL(fixedpath.substr((datpath+"/").size()));
				}
				else if(cmd == "TEXDICTION") 
				{
					std::string texpath = line.substr(space+1);
					for( size_t t = 0; t < texpath.size(); ++t) {
						texpath[t] = tolower(texpath[t]);
						if(texpath[t] == '\\') {
							texpath[t] = '/';
						}
					}
					texpath = findFileRealCase(datpath + "/" + texpath);
					std::string texname = texpath.substr(texpath.find_last_of("/")+1);
					fileLocations.insert({ texname, { false, texpath }});
				}
			}
		}
	}
}

void GTAData::loadIDE(const std::string& name)
{
	std::string lowername = name;
	for(size_t t = 0; t < lowername.size(); ++t)
	{
		lowername[t] = tolower(lowername[t]);
		if(lowername[t] == '\\') {
			lowername[t] = '/';
		}
	}
	
	ideLocations.insert({lowername, datpath+"/"+lowername});
}

void GTAData::loadCOL(const size_t zone, const std::string& name)
{
	std::cout << "COL File " << name << " for zone " << zone << std::endl;
}

void GTAData::loadIMG(const std::string& name)
{
	LoaderIMG imgLoader; 
	std::string archivePath = datpath + name;

	if (imgLoader.load(archivePath)) {
		for (int i = 0; i < imgLoader.getAssetCount(); i++) {
			auto &asset = imgLoader.getAssetInfoByIndex(i);

			std::string filename = asset.name;
			
			if(asset.size == 0)
			{
				std::cerr << "Asset " << filename << " has no size, ignoring." << std::endl;
			}
			else
			{
				// Enter the asset twice.. 
				fileLocations.insert({ filename, { true, archivePath }});
				for(size_t t = 0; t < filename.size(); ++t)
				{
					filename[t] = tolower(filename[t]);
				}
				fileLocations.insert({ filename, { true, archivePath }});
			}
		}
		archives.insert({archivePath, imgLoader});
	}
}

void GTAData::loadIPL(const std::string& name)
{
	iplLocations.insert({name, datpath + "/" + name});
}

enum ColSection {
	Unknown,
	COL,
	CAR,
	CAR3, ///> Used in GTASA, contains extra specular color
	CAR4  ///> Used in GTASA, contains quadruple colors
};

void GTAData::loadCarcols(const std::string& path)
{
	std::ifstream fstream(path.c_str());
	
	std::string line;
	ColSection currentSection = Unknown;
	while( std::getline(fstream, line)) {
		if( line.substr(0, 1) == "#") { // Comment 
			continue; 
		}
		else if( currentSection == Unknown) {
			if( line.substr(0, 3) == "col") {
				currentSection = COL;
			}
			else if( line.substr(0, 3) == "car") {
				currentSection = CAR;
			}
		}
		else if( line.substr(0, 3) == "end") {
			currentSection = Unknown;
		}
		else {
			if( currentSection == COL) {
				std::string r, g, b;
				std::stringstream ss(line);
				
				if( std::getline(ss, r, ',') && std::getline(ss, g, ',') && std::getline(ss, b)) {
					vehicleColours.push_back(glm::vec3(
						atoi(r.c_str())/255.f,
						atoi(g.c_str())/255.f,
						atoi(b.c_str())/255.f
					));
				}
			}
			else if( currentSection == CAR) {
				std::string vehicle, p, s;
				std::stringstream ss(line);
				size_t commapos = line.find(',');
				
				std::getline(ss, vehicle, ',');
				std::vector<std::pair<size_t, size_t>> colours;
				
				while( std::getline(ss, p, ',') && std::getline(ss, s, ',') ) {
					colours.push_back({ atoi(p.c_str()), atoi(s.c_str()) });
				}
				
				vehiclePalettes.insert({vehicle, colours});
			}
		}
	}
}

void GTAData::loadWeather(const std::string &path)
{
	weatherLoader.load(path);
}

void GTAData::loadWaterpro(const std::string& path)
{
	std::ifstream ifstr(path.c_str());
	
	if(ifstr.is_open()) {
		uint32_t numlevels; 
		ifstr.read(reinterpret_cast<char*>(&numlevels), sizeof(uint32_t));
		ifstr.read(reinterpret_cast<char*>(&waterHeights), sizeof(float)*48);
		
		ifstr.seekg(0x03C4);
		ifstr.read(reinterpret_cast<char*>(&visibleWater), sizeof(float)*64*64);
	}
}

void GTAData::loadWater(const std::string& path)
{
	std::ifstream ifstr(path.c_str());
	
	std::string line;
	while( std::getline(ifstr, line)) {
		if( line[0] == ';') {
			continue;
		}
		
		std::stringstream ss(line);
		
		std::string a, b, c, d, e;
		
		if( std::getline(ss, a, ',') &&  std::getline(ss, b, ',') &&  std::getline(ss, c, ',') &&  std::getline(ss, d, ',') &&  std::getline(ss, e, ',')) {
			waterRects.push_back({
				atof(a.c_str()),
				atof(b.c_str()),
				atof(c.c_str()),
				atof(d.c_str()),
				atof(e.c_str())
			});
		}
	}
}

void GTAData::loadTXD(const std::string& name) 
{
	if( loadedFiles.find(name) != loadedFiles.end()) {
		return;
	}
	
	char* file = loadFile(name);
	if(file) {
		textureLoader.loadFromMemory(file);
		delete[] file;
	}
}

void GTAData::loadDFF(const std::string& name)
{
	if( loadedFiles.find(name) != loadedFiles.end()) {
		return;
	}
	
	char *file = loadFile(name);
	if(file)
	{
		LoaderDFF dffLoader;
		models[name.substr(0, name.size() - 4)] = std::move(dffLoader.loadFromMemory(file));
		delete[] file;
	}
}

char* GTAData::loadFile(const std::string& name)
{
	if( loadedFiles.find(name) != loadedFiles.end()) {
		std::cerr << "File " << name << " already loaded!" << std::endl;
		return nullptr;
	}
	
	auto i = fileLocations.find(name);
	if(i != fileLocations.end())
	{
		if(i->second.archived)
		{
			// Find the archive
			auto ai = archives.find(i->second.path);
			if(ai != archives.end())
			{
				loadedFiles[name] = true;
				return ai->second.loadToMemory(name);
			}
			else 
			{
				std::cerr << "Archive not found " << i->second.path << std::endl;
			}
		}
		else
		{
			std::ifstream dfile(i->second.path);
			if ( ! dfile.is_open()) {
				std::cerr << "Error opening file " << i->second.path << std::endl;
				return nullptr;
			}

			dfile.seekg(0, std::ios_base::end);
			size_t length = dfile.tellg();
			dfile.seekg(0);
			char *data = new char[length];
			dfile.read(data, length);
			
			loadedFiles[name] = true;
			
			return data;
		}
	}
	else 
	{
		std::stringstream err;
		err << "Unable to locate file " << name;
		engine->logError(err.str());
		std::cerr << err.str() << std::endl;
	}
	
	return nullptr;
}