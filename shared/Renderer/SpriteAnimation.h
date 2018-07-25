#ifndef SPRITEANIMATION_H
#define SPRITEANIMATION_H

#include <string>
#include <vector>
#include <map>
#include "ClanLib-2.0/Sources/API/Core/Math/rect.h"

class SpriteSheetSurface;

/**
 * A single piece of graphic in a \c SpriteFrame.
 */
class SpriteCell
{
public:
	/**
	 * Constructs a new sprite frame cell.
	 * \param spriteName the name of the sprite to show in this cell.
	 * \param x the coordinate of the cell's left edge.
	 * \param y the coordinate of the cell's top edge.
	 * \param w the width of the cell.
	 * \param h the height of the cell.
	 * \param z determines the layer on which this cell is. Layers with
	 * higher z-value are drawn on top of layers with smaller z-values.
	 * Cells within the same z-layer are drawn in an undetermined order.
	 */
	SpriteCell(const std::string& spriteName, float x, float y, float w, float h, int z = 0);

	/**
	 * Gets the name of the sprite this cell should draw.
	 */
	const std::string& GetSpriteName() const;

	/**
	 * Gets the bounding box inside which this cell fits.
	 */
	const CL_Rectf& GetBoundingBox() const;

	/**
	 * Gets the z-layer value of this cell.
	 */
	int GetZLayer() const;

private:
	std::string m_spriteName;
	CL_Rectf m_boundingBox;
	int m_z;
};

/**
 * A sprite presentation. Consists of 0 or more \link SpriteCell \c SpriteCells \endlink
 * that form the complete picture.
 */
class SpriteFrame
{
public:
	/**
	 * Constructs a new \c SpriteFrame.
	 * \param duration specifies the duration of this frame.
	 */
	SpriteFrame(unsigned int duration);

	/**
	 * Returns the duration of this \c SpriteFrame.
	 */
	unsigned int GetDuration() const;

	/**
	 * Adds a new cell to the end of the list of cells in this frame.
	 */
	void AddCell(const SpriteCell& spriteCell);

	/**
	 * Gets the number of \link SpriteCell \c SpriteCells \endlink in this frame.
	 */
	unsigned int GetCellCount() const;

	/**
	 * Gets the requested \c SpriteCell of this frame. If \a cellIndex
	 * is out of bounds then returns \c NULL.
	 *
	 * The cells are ordered according to their z-layer values.
	 * All cells on layers &lt;n have smaller indices than cells
	 * on layer n for any z-layer value n.
	 */
	const SpriteCell* GetCell(unsigned int cellIndex) const;

	/**
	 * Gets the bounding box inside which this frame fits.
	 */
	const CL_Rectf& GetBoundingBox() const;

private:
	unsigned int m_duration;

	typedef std::vector<SpriteCell> CellList;
	CellList m_cells;

	CL_Rectf m_boundingBox;
};

/**
 * A sequence of \link SpriteFrame \c SpriteFrames \endlink forming an animation when shown one after another.
 */
class SpriteAnimation
{
public:
	SpriteAnimation();

	/**
	 * Adds a new frame to the end of the list of frames in this animation.
	 */
	void AddFrame(const SpriteFrame& spriteFrame);

	/**
	 * Gets the number of \link SpriteFrame \c SpriteFrames \endlink in this animation.
	 */
	unsigned int GetFrameCount() const;

	/**
	 * Gets the requested \c SpriteFrame of this animation. If \a frameIndex
	 * is out of bounds then returns \c NULL.
	 */
	const SpriteFrame* GetFrame(unsigned int frameIndex) const;

	/**
	 * Gets the \c SpriteFrame in this animation at position \a phase.
	 *
	 * The argument \a phase must be on the interval [0,1]. The \a phase determines
	 * a relative position on the animation. 0 signifies the beginning of the
	 * animation and 1 signifies the end.
	 *
	 * Returns \c NULL if \a phase is invalid or this animation has no frames.
	 */
	const SpriteFrame* GetFrameAtPhase(float phase) const;

	/**
	 * Gets the bounding box inside which this animation fits.
	 */
	const CL_Rectf& GetBoundingBox() const;

private:
	typedef std::vector<SpriteFrame> FrameList;
	FrameList m_frames;

	unsigned int m_totalDuration;

	CL_Rectf m_boundingBox;
};

/**
 * A collection of \link SpriteAnimation \c SpriteAnimations \endlink that use the same sprite sheet.
 * \see SpriteSheetSurface
 */
class SpriteAnimationSet
{
public:
	/**
	 * Loads a set of sprite animations from file named \a fileName.
	 * \return \c true if the loading succeeded, \c false if anything failed.
	 */
	bool LoadFile(const std::string &fileName);

	/**
	 * Gets the name of the sprite sheet resource that this set
	 * of animations uses.
	 */
	const std::string& GetSpriteSheetImageName() const;

	/**
	 * Adds a new \a animation to this set of animations with the given \a animationName.
	 *
	 * If there already is an animation with the given name in this set the previous
	 * animation is overwritten.
	 */
	void AddAnimation(const std::string& animationName, const SpriteAnimation& animation);

	/**
	 * Gets the animation with name \a animationName. Returns \c NULL if
	 * no such animation exists.
	 */
	const SpriteAnimation* GetAnimation(const std::string &animationName) const;

private:
	bool ParseFile(char *data);

	std::string m_currentSourceFile;
	std::string m_spriteSheetImageName;

	typedef std::map<std::string, SpriteAnimation> AnimationMap;
	AnimationMap m_animations;

};

#endif // SPRITEANIMATION_H
