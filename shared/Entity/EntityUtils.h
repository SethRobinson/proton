#ifndef EntityUtils_h__
#define EntityUtils_h__
//Misc junk for making the entity code cleaner
#include "BaseApp.h"
#include "Entity.h"

#include "Entity/OverlayRenderComponent.h"
#include "Entity/TextRenderComponent.h"
#include "Entity/TouchHandlerComponent.h"
#include "Entity/TouchStripComponent.h"
#include "Entity/Button2DComponent.h"
#include "Entity/FocusInputComponent.h"
#include "Entity/FocusUpdateComponent.h"
#include "Entity/FocusRenderComponent.h"
#include "Entity/EntityUtils.h"
#include "Entity/InterpolateComponent.h"
#include "Entity/ProgressBarComponent.h"
#include "Entity/UnderlineRenderComponent.h"
#include "Entity/TapSequenceDetectComponent.h"
#include "Entity/TyperComponent.h"
#include "Entity/RectRenderComponent.h"
#include "Entity/ScrollComponent.h"
#include "Entity/ScrollBarRenderComponent.h"
#include "Entity/TextBoxRenderComponent.h"
#include "Entity/InputTextRenderComponent.h"
#include "Entity/FilterInputComponent.h"
#include "Entity/TrailRenderComponent.h"
#include "Entity/RenderClipComponent.h"
#include "Entity/SelectButtonWithCustomInputComponent.h"
#include "Entity/SliderComponent.h"
#include "Entity/EmitVirtualKeyComponent.h"
#include "Entity/RenderScissorComponent.h"

const int C_IPAD_PPI = 132;
const int C_IPHONE_PPI = 163;


Entity * CreateTextLabelEntity(Entity *pParentEnt, string name, float x, float y, string text);
Entity * CreateTextButtonEntity(Entity *pParentEnt, string name, float x, float y, string text, bool bUnderline = true);
/**
 * Creates a new \c Entity and adds an \c OverlayRenderComponent to it.
 *
 * If \a pParentEnt is not \c NULL the new \c Entity is added as a child to that \c Entity.
 * If \a pParentEnt is \c NULL then the new \c Entity will have no parent.
 *
 * The argument \a name sets the name of the new \c Entity.
 *
 * Arguments \a x and \a y specify the position of the new \c Entity. The position
 * of the \c OverlayRenderComponent follows the position of its parent \c Entity
 * so these also set the position of that.
 *
 * \see OverlayRenderComponent
 *
 * \return The newly created \c Entity or \c NULL if the creation failed for any reason.
 */
Entity * CreateOverlayEntity(Entity *pParentEnt, string name, string fileName, float x, float y, bool bAddBasePath = true);
/**
 * Like \c CreateOverlayEntity() but additionally adds a \c TouchHandlerComponent and a
 * \c Button2DComponent to the created \c Entity too.
 *
 * \see TouchHandlerComponent, Button2DComponent
 */
Entity * CreateOverlayButtonEntity(Entity *pParentEnt, string name, string fileName, float x, float y);
Entity * CreateOverlayRectEntity(Entity *pParent, CL_Rectf posAndBoundsRect, uint32 color, RectRenderComponent::eVisualStyle style = RectRenderComponent::STYLE_NORMAL);
Entity * CreateOverlayRectEntity(Entity *pParent, CL_Vec2f vPos, CL_Vec2f vBounds, uint32 color, RectRenderComponent::eVisualStyle style = RectRenderComponent::STYLE_NORMAL);
Entity * CreateButtonHotspot(Entity *pParentEnt, string name, CL_Vec2f vPos, CL_Vec2f vBounds, Button2DComponent::eButtonStyle buttonStyle = Button2DComponent::BUTTON_STYLE_CLICK_ON_TOUCH);
/**
 * Creates a new \c Entity and adds a \c TextBoxRenderComponent to it.
 *
 * The \a pParent will be the parent \c Entity for the new \c Entity. It mustn't be \c NULL.
 * The name of the new \c Entity is set to \a entName.
 *
 * The new \c Entity's position ("pos2d") and size ("size2d") are
 * set to \a vPos and \a vTextAreaSize respectively.
 *
 * The following values are set to the created \c TextBoxRenderComponent:
 * - the text is set to \a msg
 * - the scale of the text is set to \a scale
 * - the text alignment in set to \a textAlignment
 *
 * \see TextBoxRenderComponent
 * \return The newly created \c Entity.
 */
Entity * CreateTextBoxEntity(Entity *pParent, string entName, CL_Vec2f vPos, CL_Vec2f vTextAreaSize, string msg, float scale = 1.0f, eAlignment textAlignment = ALIGNMENT_UPPER_LEFT);

/**
Like above, but puts the TextBoxRender component under a ScrollComponent.
It actually creates the structure scroll->scroll_child->entName.  Function returns entName, but keep in mind it's
inside two other entities.

It will automatically resize the scroll bars when it notices "text" changing in the TextBoxRender component.
*/
Entity * CreateScrollingTextBoxEntity(Entity *pParent, string entName, CL_Vec2f vPos, CL_Vec2f vTextAreaSize, string msg, float scale = 1.0f, eAlignment textAlignment = ALIGNMENT_UPPER_LEFT);

Entity * CreateInputTextEntity(Entity *pParentEnt, string name, float x, float y, string text, float sizeX=0, float sizeY=0);

//Checkbox functionality without needing a new component, purely done with proton messaging stuff
Entity * CreateCheckbox(Entity *pBG, string name, string text, float x, float y, bool bChecked, eFont fontID = FONT_SMALL, float fontScale = 1.0f,bool unclickable=false);
bool IsCheckboxChecked(Entity *pEnt); //used with
void SetCheckBoxChecked(Entity *pEnt, bool bChecked, bool bShowAnim = true); //anim means that little bob thing
void OnCheckboxToggle(VariantList *pVList);

void DisableAllButtonsEntity(Entity *pEnt, bool bRecursive = true);
void EnableAllButtonsEntity(Entity *pEnt, bool bRecursive = true, int delayBeforeActionMS = 0, eTimingSystem timing = GetTiming());

Entity * EnableEntityButtonByName(const string &entityName, Entity *pRootEntity = GetEntityRoot()); //TODO: Too much duplicated code with below
Entity * DisableEntityButtonByName(const string &entityName, Entity *pRootEntity = GetEntityRoot());
EntityComponent * SlideScreen(Entity *pEnt, bool bIn, int speedMS = 500, int delayToStartMS = 0);
EntityComponent * SlideScreenBackwards(Entity *pEnt, bool bIn, int speedMS = 500, int delayToStartMS = 0);
EntityComponent * SlideScreenVertical(Entity *pEnt, bool bIn, int speedMS = 500, int delayToStartMS = 0);

bool IsEntityBobbing(Entity *pEnt); //returns true if BobEntity or OneTimeBobEntity is active on this entity
void BobEntity(Entity *pEnt, float bobAmount = 3, int delayBeforeBob = 0, int durationOfEachBobMS = 1000); //make an entity vertically "bounce" forever
void BobEntityStop(Entity *pEnt); //stop bouncing
void OneTimeBobEntity(Entity *pEnt, float bobAmount = -10, int delayBeforeBob = 0, int durationMS = 100);
/**
 * Ensures that the given \c Entity has needed focus \link EntityComponent \c EntityComponents \endlink.
 *
 * Three components will be added to \a pEnt, one instance of each of these classes:
 * - FocusUpdateComponent
 * - FocusRenderComponent
 * - FocusInputComponent
 *
 * If any focus component is already on the \c Entity they are not added again. Hence it's safe to
 * call this method multiple times for an \c Entity.
 *
 * \param pEnt this is the \c Entity where the components are added. Mustn't be \c NULL.
 * \param bAlsoLinkMoveMessages if \c true, calls the \link FocusInputComponent::LinkMoveMessages()
 *        \c LinkMoveMessages() \endlink method for the added \c FocusInputComponent.
 * \param delayInputMS specifies a delay in milliseconds after which the \c FocusInputComponent is added to the \c Entity.
 * \param updateAndRenderDelay specifies a delay in milliseconds after which the \c FocusUpdateComponent and
 *        \c FocusRenderComponent are added to the \c Entity.
 */
void AddFocusIfNeeded(Entity *pEnt, bool bAlsoLinkMoveMessages = false, int delayInputMS = 0, int updateAndRenderDelay = 0); //add input, think, and render focuses to an entity (if they don't exist)
void AddInputMovementFocusIfNeeded(Entity *pEnt);
/**
 * Commands a \c FocusInputComponent in \a pEnt to also recognize the raw keyboard input messages.
 *
 * The \c FocusInputComponent is searched for in \a pEnt and its parent \link Entity
 * <tt>Entities</tt>\endlink. If no \c FocusInputComponent is found then this method has
 * no effect.
 *
 * \see FocusInputComponent
 */
void EnableRawMessageFocusInput(Entity *pEnt);
void RemoveFocusIfNeeded(Entity *pEnt); //remove input, think, and render focuses from an entity
void RemoveInputFocusIfNeeded(Entity *pEnt); //just removes input

void FadeOutEntity(Entity *pEnt, bool bRecursive=true, int timeMS=300, int delayBeforeFadingMS = 0, eTimingSystem timing = GetTiming());
void FadeOutAndKillEntity(Entity *pEnt, bool bRecursive = true, int timeMS=300, int delayBeforeFadingMS = 0, eTimingSystem timing = GetTiming());
void FadeOutAndKillChildrenEntities(Entity *pEnt, int timeMS=300, int delayBeforeFadingMS = 0,  eTimingSystem timing = GetTiming());
void FadeInEntity(Entity *pEnt, bool bRecursive=true, int timeMS=300, int delayBeforeFadingMS = 0, float fadeTarget = 1.0f, eTimingSystem timing = GetTiming());
void KillEntity(Entity *pEnt, int timeMS = 0, eTimingSystem timing = GetTiming());
EntityComponent * PulsateColorEntity(Entity *pEnt, bool bRecursive, unsigned int color, unsigned int pulsateSpeedMS = 250);

//modifies an existing textlabel to 'type' itself out.  Deletes any pre-existing Typer effect.  If textToAddByTyping is blank, the entire
//pre-existing text on the label/textbox is typed
EntityComponent * TypeTextLabelEntity(Entity *pEnt, int delayBeforeActionMS = 0, uint32 textTypeSpeedMS = 50,
									  TyperComponent::eMode = TyperComponent::MODE_ONCE_AND_REMOVE_SELF, string textToAddByTyping = "");

/**
 * Adds a position animation to an \c Entity.
 * If there is a previous position animation running it is removed first.
 *
 * The animation starts from the current position of the \c Entity offset by \a offset.
 * The animation will end to the current position of the \c Entity.
 *
 * \param pEnt the \c Entity to animate.
 * \param offset the offset from where the animation starts.
 * \param speedMS the duration of the animation in milliseconds.
 * \param interpolateType the type of the interpolation.
 * \param delayBeforeActionMS delay in milliseconds before the animation is started.
 *        If set to 0 the animation is started immediately.
 * \return the \c EntityComponent that performs the animation.
 */
EntityComponent * ZoomToPositionFromThisOffsetEntity(Entity *pEnt, CL_Vec2f offset, unsigned int speedMS, eInterpolateType interpolateType = INTERPOLATE_SMOOTHSTEP,  int delayBeforeActionMS = 0);
/**
 * Adds a position animation to an \c Entity.
 * If there is a previous position animation running it is removed first.
 *
 * The animation starts from the current position of the \c Entity.
 * The end position is determined by \a vPos.
 *
 * \param pEnt the \c Entity to animate.
 * \param vPos the end position of the animation.
 * \param speedMS the duration of the animation in milliseconds.
 * \param interpolateType the type of the interpolation.
 * \param delayBeforeActionMS delay in milliseconds before the animation is started.
 *        If set to 0 the animation is started immediately.
 * \return the \c EntityComponent that performs the animation.
 */
EntityComponent * ZoomToPositionEntity(Entity *pEnt, CL_Vec2f vPos, unsigned int speedMS, eInterpolateType interpolateType = INTERPOLATE_SMOOTHSTEP,  int delayBeforeActionMS = 0);
/**
 * Adds a position animation to an \c Entity.
 * If there is a previous position animation running it is removed first.
 *
 * The animation starts from the position determined by \a vPos.
 * The animation will end to the current position of the \c Entity.
 *
 * \param pEnt the \c Entity to animate.
 * \param vPos the start position of the animation.
 * \param speedMS the duration of the animation in milliseconds.
 * \param interpolateType the type of the interpolation.
 * \param delayBeforeActionMS delay in milliseconds before the animation is started.
 *        If set to 0 the animation is started immediately.
 * \return the \c EntityComponent that performs the animation.
 */
EntityComponent * ZoomFromPositionEntity(Entity *pEnt, CL_Vec2f vPos, unsigned int speedMS, eInterpolateType interpolateType = INTERPOLATE_SMOOTHSTEP,  int delayBeforeActionMS = 0);
/**
 * Adds a position animation to an \c Entity.
 * If there is a previous position animation running it is removed first.
 *
 * The animation starts from the current position of the \c Entity.
 * The animation will end to the current position of the \c Entity offset by \a offset.
 *
 * \param pEnt the \c Entity to animate.
 * \param offset the offset to where the animation ends.
 * \param speedMS the duration of the animation in milliseconds.
 * \param interpolateType the type of the interpolation.
 * \param delayBeforeActionMS delay in milliseconds before the animation is started.
 *        If set to 0 the animation is started immediately.
 * \return the \c EntityComponent that performs the animation.
 */
EntityComponent * ZoomToPositionOffsetEntity(Entity *pEnt, CL_Vec2f offset, unsigned int speedMS, eInterpolateType interpolateType = INTERPOLATE_SMOOTHSTEP,  int delayBeforeActionMS = 0);
/**
 * Like ZoomToPositionOffsetEntity() but doesn't remove any animations currently running.
 */
EntityComponent * ZoomToPositionOffsetEntityMulti(Entity *pEnt, CL_Vec2f offset, unsigned int speedMS, eInterpolateType interpolateType = INTERPOLATE_SMOOTHSTEP,  int delayBeforeActionMS = 0);
/**
 * Like ZoomToPositionEntity() but doesn't remove any animations currently running.
 */
EntityComponent * ZoomToPositionEntityMulti(Entity *pEnt, CL_Vec2f vPos, unsigned int speedMS, eInterpolateType interpolateType = INTERPOLATE_SMOOTHSTEP,  int delayBeforeActionMS = 0);

/**
 * Adds a scale animation to an \c Entity.
 *
 * The animation starts from the current scale of the \c Entity.
 * \param pEnt the \c Entity to animate.
 * \param vScale the end scale of the animation.
 * \param speedMS the duration of the animation in milliseconds.
 * \param interpolateType the type of the interpolation.
 * \param delayBeforeActionMS delay in milliseconds before the animation is started.
 *        If set to 0 the animation is started immediately.
 * \return the \c EntityComponent that performs the animation.
 */
EntityComponent * ZoomToScaleEntity(Entity *pEnt, CL_Vec2f vScale, unsigned int speedMS, eInterpolateType interpolateType = INTERPOLATE_SMOOTHSTEP, int delayBeforeActionMS = 0);

EntityComponent * MorphToVec2Entity(Entity *pEnt, string targetVar, CL_Vec2f vTargetSize, unsigned int speedMS, eInterpolateType interpolateType = INTERPOLATE_SMOOTHSTEP,  int delayBeforeActionMS = 0);
//like above, but allows more than one to be layered instead of assuming you want to ignore the previous one
EntityComponent * MorphToVec2EntityMulti(Entity *pEnt, string targetVar, CL_Vec2f vTargetSize, unsigned int speedMS, eInterpolateType interpolateType = INTERPOLATE_SMOOTHSTEP,  int delayBeforeActionMS = 0);

EntityComponent * MorphToSizeEntity(Entity *pEnt, CL_Vec2f vTargetSize, unsigned int speedMS, eInterpolateType interpolateType = INTERPOLATE_SMOOTHSTEP,  int delayBeforeActionMS = 0);
void MorphToColorEntity(Entity *pEnt, bool bRecursive, int timeMS, unsigned int color, int delayBeforeActionMS=0, bool bAllowMultipleAtOnce = false);
EntityComponent * MorphToFloatEntity(Entity *pEnt, string targetVar, float target, unsigned int speedMS, eInterpolateType interpolateType = INTERPOLATE_SMOOTHSTEP,  int delayBeforeActionMS = 0);
EntityComponent * MorphToFloatComponent(EntityComponent *pTargetComp, string targetVar, float target, unsigned int speedMS, eInterpolateType interpolateType = INTERPOLATE_SMOOTHSTEP,  int delayBeforeActionMS = 0);
EntityComponent * MorphToVec2Component(EntityComponent *pTargetComp, string targetVar, CL_Vec2f vTargetSize, unsigned int speedMS, eInterpolateType interpolateType = INTERPOLATE_SMOOTHSTEP,  int delayBeforeActionMS = 0);

void FlashStartEntity(Entity *pEnt, int flashSpeedMS = 250);
void FlashStopEntity(Entity *pEnt);
void FlashOnceEntity(Entity *pEnt, int flashSpeedMS);

/**
 * Sets the filename of an \c OverlayRenderComponent found from \a pEntWithOverlayComponent.
 *
 * The entity must contain an \c OverlayRenderComponent named "OverlayRender" (which is
 * the default name for that component). Otherwise an assert occurs.
 *
 * The "fileName" attribute of the \c OverlayRenderComponent is set to \a imageFileName.
 *
 * If \a delayBeforeActionMS is greater than 0 then the "fileName" attribute is set only after
 * a delay of \a delayBeforeActionMS milliseconds.
 *
 * \return the \c OverlayRenderComponent that got modified.
 */
EntityComponent * SetOverlayImageEntity(Entity *pEntWithOverlayComponent, string imageFileName, uint32 delayBeforeActionMS = 0, bool bAddBasePath = true);
/**
 * Gets the filename of an \c OverlayRenderComponent found from \a pEntWithOverlayComponent.
 *
 * The entity should contain an \c OverlayRenderComponent named "OverlayRender" (which is
 * the default name for that component). The returned value is the current value of the
 * "fileName" attribute in that component.
 *
 * Returns an empty string if:
 * - no component named "OverlayRender" is found in the entity, or
 * - the found component has no attribute named "fileName", or
 * - the attribute really is an empty string
 */
std::string GetOverlayImageEntity(Entity *pEntWithOverlayComponent);

EntityComponent * SetupAnimEntity(Entity *pEnt, uint32 frameCountX, uint32 frameCountY = 1, int curFrameX = -1, int curFrameY = -1); //-1 means don't set it
void AnimateEntity(Entity *pEnt, int startFrame, int endFrame, int animSpeedMS, InterpolateComponent::eOnFinish type, int delayToStartMS);
void AnimateStopEntity(Entity *pEnt, int delayToStartMS);
void AnimateStopEntityAndSetFrame(Entity *pEnt, int delayToStartMS, int frameX, int frameY);
void AnimateEntitySetMirrorMode(Entity *pEnt, bool flipX, bool flipY);

void ScaleEntity(Entity *pEnt, float scaleStart, float scaleEnd, int timeMS, int delayBeforeStartingMS = 0, eInterpolateType interpolationType = INTERPOLATE_SMOOTHSTEP); //send -1 for scaleStart to use current scale as start
void ScaleEntity(Entity *pEnt, CL_Vec2f vScaleStart, CL_Vec2f vScaleEnd, int timeMS, int delayBeforeStartingMS = 0, eInterpolateType interpolationType = INTERPOLATE_SMOOTHSTEP); //send -1 for scaleStart to use current scale as start

void FadeEntity(Entity *pEnt, bool bRecursive, float alpha, int timeMS, int delayBeforeFadingMS = 0, bool bAllowMultipleFadesActiveAtOnce = false);
Entity * FadeScreen( Entity *pParent, float defaultStartAlpha, float targetAlpha, int fadeDurationMS, bool bDeleteWhenDone);

//if pVStartingPos isn't null, the position where the rect starts as compared to the 0,0 of the parent entity is returned in it
CL_Rectf MeasureEntityAndChildren(Entity *pEnt, CL_Vec2f *pVStartingPos = NULL, bool bFirst = true);

//simple helpers to set parameters
void SetupTextEntity(Entity *pEnt, eFont fontID, float scale=0); //0 means no change
void SetAlignmentEntity(Entity *pEnt, eAlignment align);
eAlignment GetAlignmentEntity(Entity *pEnt);
void SetTouchPaddingEntity(Entity *pEnt, CL_Rectf padding); //left, top, right, bottom
EntityComponent * SetButtonVisualStyleEntity(Entity *pEnt, Button2DComponent::eVisualStyle style);
EntityComponent * SetButtonStyleEntity(Entity *pEnt, Button2DComponent::eButtonStyle style);
void SetButtonClickSound(Entity *pEnt, string fileName);
void SetAlphaEntity(Entity *pEnt, float alpha);
void PreloadKeyboard(OSMessage::eParmKeyboardType keyboardType=OSMessage::PARM_KEYBOARD_TYPE_ASCII);
void SendFakeInputMessageToEntity(Entity *pEnt, eMessageType msg, CL_Vec2f vClickPos, int delayBeforeStartingMS = 0);
void SendFakeButtonPushToEntity(Entity *pEntity, int timeMS);
void SetButtonRepeatDelayMS(Entity *pEnt, uint32 delayMS);
void LightBarOnChange(VariantList *pVList);
void SetupLightBarSelect(Entity *pBG, string entNamePrefix, int defaultOption, uint32 color);
void ResizeScrollBounds(VariantList *pVList);
void DisableHorizontalScrolling(Entity *pEnt);
/**
 * Sets the progress (position) of a \c ScrollComponent.
 * The component needs to be found from \a pEnt.
 * \see ScrollComponent for details.
 */
void SetScrollProgressEntity(Entity *pEnt, const CL_Vec2f &progress);

CL_Vec2f GetScrollProgressEntity(Entity *pEnt);

void SetDisabledOnAllComponentsRecursively(Entity *pEnt, bool bDisabled);
EntityComponent * DisableComponentByName(Entity *pEnt, const string &compName, int delayBeforeActionMS=0); //applicable to components that check a var named "disabled", like Button2D or InputTextRender
EntityComponent * EnableComponentByName(Entity *pEnt, const string &compName, int delayBeforeActionMS=0); //applicable to components that check a var named "disabled", like Button2D or InputTextRender
CL_Vec2f ConvertEntityClickToScreenCoords(CL_Vec2f pt, Entity *pEnt);
void GetUsedTextures(vector<string> &usedTextures, Entity *pEnt);
void DestroyUnusedTextures();
bool EntityRetinaRemapIfNeeded(Entity *pEnt, bool bAdjustPosition = true, bool bAdjustScale = false, bool bApplyToIpadAlso = true, bool bPerserveAspectRatio = false); //moves and scale entities to fit a retina screen like an iphone screen
/**
 * Sets the scale of \a pEnt such that the resulting size of it will be \a vDestSize.
 *
 * If the \a bPreserveAspectRatio is set to \c true the resulting scale is uniform.
 * This means that \a pEnt's appearance will preserve its aspect ratio.
 * If \a pEnt is taller than it's wide \a vDestSize.y is ignored and the resulting
 * width of \a pEnt will be \a vDestSize.x. The height of \a pEnt will be whatever
 * it needs to be to preserve the aspect ratio. Similarly if \a pEnt is wider than
 * it's tall <b>or it's square</b> \a vDestSize.x is ignored and the resulting
 * height of \a pEnt will be \a vDestSize.y. The width of \a pEnt will be whatever
 * it needs to be to preserve the aspect ratio.
 * To respect the the opposite axis, bPreserveOtherAxis can be set to true.
 */
void EntitySetScaleBySize(Entity *pEnt, CL_Vec2f vDestSize, bool bPreserveAspectRatio = false, bool bPreserveOtherAxis = false);
void EntitySetScaleBySizeAndAspectMode(Entity *pEnt, CL_Vec2f vDestSize, eAspect aspectMode);
void EntityScaleiPad(Entity *pEnt, bool bPerserveAspectRatio); //on ipad size does nothing, on anything else it scales to fit the same screen ratio.
void EntityAdjustScaleSoPhysicalSizeMatches(Entity *pEnt, int ppiToMatch = C_IPAD_PPI, float powerMult = 1.0f); //so a 1 inch button on ipad1 is also 1 inch on a retina iphone or anything else.  powerMult 0.5 means weaken the changes by half
EntityComponent * AddHotKeyToButton(Entity *pEnt, uint32 keycode);
EntityComponent * MakeButtonEmitVirtualGameKey(Entity *pEnt, uint32 keycode);
EntityComponent * CreateSlider(Entity *pBG, float x, float y, float sizeX, string buttonFileName, string left, string middle, string right);
void AdjustGUIElementForWindowView(Entity *pEnt, CL_Rectf r, float rotation);
void ManuallySetAlignmentEntity(Entity *pEnt, eAlignment alignment); //takes into account rotation as in screen rotation
bool IsDisabledEntity(Entity *pEnt);
void RemovePaddingEntity(Entity *pEnt);
void FakeClickAnEntity(Entity *pEnt, int delayBeforeStartingMS = 0); //good for pretending to click a Button2D programically or something
bool FakeClickAnEntityByName(Entity *pEntitySearchRoot, const string name);
bool EntityIsOnScreen(Entity *pEnt); //returns true if any part of an entity overlaps the visible screen.  (reads pos2d and size2d to test)

EntityComponent * SetTextEntity(Entity *pEntWithTextComponent, const string &text);
//will locate (by name) and modify text on an entity created with CreateTextLabelEntity() (or any entity with a TextRenderer component in it)
Entity * SetTextEntityByName(const string &entityName, string text, Entity *pRootEntity = GetEntityRoot());

//helpers for the lazy.. the reason I don't return a const pointer to the value is I tend to want to operate on the return vars right away.. -Seth
CL_Vec2f GetSize2DEntity(Entity *pEnt);
void SetSize2DEntity(Entity *pEnt, const CL_Vec2f &vSize);
CL_Vec2f GetImageSize2DEntity(Entity *pEnt); //reads the raw image size of the image if this ent has an OverlayRenderComponent in it
CL_Vec2f GetPos2DEntity(Entity *pEnt);
void SetPos2DEntity(Entity *pEnt, const CL_Vec2f &vPos);
CL_Vec2f GetScale2DEntity(Entity *pEnt);
void SetScale2DEntity(Entity *pEnt, const CL_Vec2f &vScale);
void SetVisibleEntity(Entity *pEnt, bool bVisible);
CL_Vec2f GetScreenPos2DEntity(Entity *pEnt, CL_Vec2f vRecursivePosToAdd = CL_Vec2f(0,0)); //the post transformed position, it recursively adds all parent positions to figure it out, slow

void SetProgressBarPercent(Entity *pEnt, float progressPercent);
float EnforceMinimumFontLineToScreenRatio(eFont fontID, float fontScale, float minLineToScreenRatio);

float EnforceMinimumFontLineToScreenRatioAllowBig(eFont fontID, float fontScale, float minLineToScreenRatio);
//Copies one or more properties by name from one entity to another
void CopyPropertiesToEntity(Entity *pToEnt, Entity *pFromEnt, const string varName1, const string varName2="", const string varName3="");

//Draw a text message on the screen, then (by default) kill it 3 seconds later.  Useful for quick debugging messages sometimes
void ShowTextMessage(string msg, int timeMS=1500, int delayBeforeStartingMS = 0);

bool EntityHasInputFocus(Entity *pEnt); //returns true if this entity can currently receive input
void SetupEntityToEatInput(Entity *pEnt); //assign to a background, and no clicks will live past it
void ActivateTextInputEntity(Entity *pEnt); //gives focus to an entity that has an InputTextRender component in it
void SetTextShadowColor(Entity *pEnt, uint32 color);
void GetFontAndScaleToFitThisLinesPerScreenY(eFont *pFontIDOut, float *pFontScaleOut, float desiredLinesPerScreenY);
void GetFontAndScaleToFitThisPixelHeight(eFont *pFontIDOut, float *pFontScaleOut, float heightPixels);
void GetFontAndScaleToFitThisStringInWidthPixels(eFont *pFontIDOut, float *pFontScaleOut, string text, float desiredWidth);
void SetEntityOverlayToUnloadImageOnKill(Entity *pImage);
#endif // EntityUtils_h__
