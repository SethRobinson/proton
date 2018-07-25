#include "PlatformPrecomp.h"

#include "SpriteAnimation.h"
#include "Renderer/SpriteSheetSurface.h"

#include "util/rapidxml/rapidxml.hpp"

using namespace std;
using namespace rapidxml;

SpriteCell::SpriteCell(const std::string &spriteName, float x, float y, float w, float h, int z) :
	m_spriteName(spriteName),
	m_boundingBox(x, y, x + w, y + h),
	m_z(z)
{
}

const string& SpriteCell::GetSpriteName() const
{
	return m_spriteName;
}

const CL_Rectf& SpriteCell::GetBoundingBox() const
{
	return m_boundingBox;
}

int SpriteCell::GetZLayer() const
{
	return m_z;
}



SpriteFrame::SpriteFrame(unsigned int duration) :
	m_duration(duration)
{
}

unsigned int SpriteFrame::GetDuration() const
{
	return m_duration;
}

void SpriteFrame::AddCell(const SpriteCell& spriteCell)
{
	CellList::iterator it(m_cells.begin());

	while (it != m_cells.end())
	{
		if ((*it).GetZLayer() > spriteCell.GetZLayer())
		{
			break;
		}
		it++;
	}

	m_cells.insert(it, spriteCell);

	if (m_cells.size() == 1)
	{
		m_boundingBox = spriteCell.GetBoundingBox();
	} else
	{
		m_boundingBox.bounding_rect(spriteCell.GetBoundingBox());
	}
}

unsigned int SpriteFrame::GetCellCount() const
{
	return m_cells.size();
}

const SpriteCell* SpriteFrame::GetCell(unsigned int cellIndex) const
{
	if (cellIndex < m_cells.size())
	{
		return &m_cells.at(cellIndex);
	}

	return NULL;
}

const CL_Rectf& SpriteFrame::GetBoundingBox() const
{
	return m_boundingBox;
}



SpriteAnimation::SpriteAnimation() :
	m_totalDuration(0)
{
}

void SpriteAnimation::AddFrame(const SpriteFrame& spriteFrame)
{
	m_frames.push_back(spriteFrame);
	m_totalDuration += spriteFrame.GetDuration();

	if (m_frames.size() == 1)
	{
		m_boundingBox = spriteFrame.GetBoundingBox();
	} else
	{
		m_boundingBox.bounding_rect(spriteFrame.GetBoundingBox());
	}
}

unsigned int SpriteAnimation::GetFrameCount() const
{
	return m_frames.size();
}

const SpriteFrame* SpriteAnimation::GetFrame(unsigned int frameIndex) const
{
	if (frameIndex < m_frames.size())
	{
		return &m_frames.at(frameIndex);
	}

	return NULL;
}

const SpriteFrame* SpriteAnimation::GetFrameAtPhase(float phase) const
{
	if (phase < 0.0f || phase > 1.0f)
	{
		return NULL;
	}
	if (phase == 1.0f)
	{
		return &m_frames.back();
	}

	const unsigned int phaseInTotalDuration = phase * m_totalDuration;

	unsigned int cumulativeDuration = 0;
	unsigned int previousCumulativeDuration = 0;

	for (FrameList::const_iterator it(m_frames.begin()); it != m_frames.end(); it++)
	{
		cumulativeDuration += (*it).GetDuration();

		if (previousCumulativeDuration <= phaseInTotalDuration && phaseInTotalDuration < cumulativeDuration)
		{
			return &(*it);
		}

		previousCumulativeDuration = cumulativeDuration;
	}

	return NULL;
}

const CL_Rectf& SpriteAnimation::GetBoundingBox() const
{
	return m_boundingBox;
}



bool SpriteAnimationSet::LoadFile(const string &fileName)
{
	if (fileName.empty())
	{
		return true;
	}

	FileInstance file(fileName);
	if (!file.IsLoaded())
	{
		return false;
	}

	m_currentSourceFile = fileName;
	if (!ParseFile(file.GetAsChars()))
	{
		LogError("Loading sprite animation file %s failed!", fileName.c_str());
		m_currentSourceFile.clear();
		return false;
	}

	return true;
}

const string& SpriteAnimationSet::GetSpriteSheetImageName() const
{
	return m_spriteSheetImageName;
}

void SpriteAnimationSet::AddAnimation(const std::string& animationName, const SpriteAnimation& animation)
{
	m_animations[animationName] = animation;
}

const SpriteAnimation* SpriteAnimationSet::GetAnimation(const string &animationName) const
{
	AnimationMap::const_iterator it = m_animations.find(animationName);
	if (it != m_animations.end())
	{
		return &it->second;
	}

	return NULL;
}



class DarkFunctionParser
{
public:
	DarkFunctionParser(SpriteAnimationSet& animationSet, const std::string &resourceDir) :
		m_animationSet(animationSet),
		m_resourceDir(resourceDir),
		m_spriteSheet(NULL)
	{
	}

	void ParseSpriteSheetDirNode(const xml_node<>& dirNode)
	{
		string dirName(dirNode.first_attribute("name")->value());
		StringReplace(string("/"), string(""), dirName);
		m_dirNames.push(dirName);

		for (xml_node<> *childNode = dirNode.first_node(); childNode; childNode = childNode->next_sibling())
		{
			if (string("dir") == childNode->name())
			{
				ParseSpriteSheetDirNode(*childNode);
			} else if (string("spr") == childNode->name())
			{
				float x = atof(childNode->first_attribute("x")->value());
				float y = atof(childNode->first_attribute("y")->value());
				float w = atof(childNode->first_attribute("w")->value());
				float h = atof(childNode->first_attribute("h")->value());

				string sprName(childNode->first_attribute("name")->value());
				StringReplace(string("/"), string(""), sprName);
				m_spriteSheet->AddFrame(m_dirNames.value() + sprName, CL_Rect(x, y, x + w, y + h));
			}
		}

		m_dirNames.pop();
	}

	bool ParseSpriteSheetFile(const xml_document<>& doc)
	{
		xml_node<> *imgNode = doc.first_node("img");
		char *imageName = imgNode->first_attribute("name")->value();

		m_spriteSheetImagePath = m_resourceDir + imageName;

		m_spriteSheet = GetResourceManager()->GetSurfaceResource<SpriteSheetSurface>(m_spriteSheetImagePath);
		if (m_spriteSheet == NULL) {
			LogError("Resource manager didn't return a sprite sheet surface with name %s.", m_spriteSheetImagePath.c_str());
			return false;
		}

		xml_node<> *dirNode = imgNode->first_node("definitions")->first_node("dir");
		ParseSpriteSheetDirNode(*dirNode);

		return true;
	}

	bool ParseAnimFile(const xml_document<>& doc)
	{
		xml_node<> *animationsNode = doc.first_node("animations");
		char *spriteSheetName = animationsNode->first_attribute("spriteSheet")->value();
		string completeSpriteSheetPath = m_resourceDir + spriteSheetName;

		FileInstance file(completeSpriteSheetPath);
		if (!file.IsLoaded())
		{
			LogError("Can't find the sprite sheet file %s", completeSpriteSheetPath.c_str());
			return false;
		}

		xml_document<> spriteDoc;
		spriteDoc.parse<0>(file.GetAsChars());
		if (!ParseSpriteSheetFile(spriteDoc))
		{
			return false;
		}

		for (xml_node<> *animNode = animationsNode->first_node("anim"); animNode; animNode = animNode->next_sibling("anim"))
		{
			string animationName(animNode->first_attribute("name")->value());
			SpriteAnimation animation;

			for (xml_node<> *cellNode = animNode->first_node("cell"); cellNode; cellNode = cellNode->next_sibling("cell"))
			{
				SpriteFrame frame(atoi(cellNode->first_attribute("delay")->value()));

				for (xml_node<> *spriteNode = cellNode->first_node("spr"); spriteNode; spriteNode = spriteNode->next_sibling("spr"))
				{
					float x = atof(spriteNode->first_attribute("x")->value());
					float y = atof(spriteNode->first_attribute("y")->value());
					// darkFunction editor has an inverted style of interpreting the z-values.
					// Taking the negated value of the z puts them in the correct order.
					int z = -atoi(spriteNode->first_attribute("z")->value());

					string frameName(spriteNode->first_attribute("name")->value());
					StringReplace(string("/"), string(""), frameName);
					CL_Vec2f frameSize(m_spriteSheet->GetFrameSize(frameName));

					frame.AddCell(SpriteCell(frameName, x - frameSize.x / 2, y - frameSize.y / 2, frameSize.x, frameSize.y, z));
				}

				animation.AddFrame(frame);
			}

			m_animationSet.AddAnimation(animationName, animation);
		}

		return true;
	}

	std::string m_spriteSheetImagePath;

private:
	class StringStack
	{
	public:
		void push(const std::string &val)
		{
			m_framesizes.push_back(val.size());
			m_value += val;
		}

		void pop()
		{
			if (!m_framesizes.empty())
			{
				m_value.erase(m_value.size() - m_framesizes.back());
				m_framesizes.pop_back();
			}
		}

		const std::string& value() const
		{
			return m_value;
		}

	private:
		std::vector<size_t> m_framesizes;
		std::string m_value;
	};

	SpriteAnimationSet &m_animationSet;
	std::string m_resourceDir;
	SpriteSheetSurface *m_spriteSheet;
	StringStack m_dirNames;

};


bool SpriteAnimationSet::ParseFile(char *data)
{
	xml_document<> doc;
	doc.parse<0>(data);

	bool parseResult = false;

	if (string("animations") == doc.first_node()->name())
	{
		DarkFunctionParser parser(*this, GetPathFromString(m_currentSourceFile));
		parseResult = parser.ParseAnimFile(doc);
		if (parseResult)
		{
			m_spriteSheetImageName = parser.m_spriteSheetImagePath;
		}
	}

	return parseResult;
}
