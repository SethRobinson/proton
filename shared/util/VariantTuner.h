#ifndef VARIANTTUNER_H
#define VARIANTTUNER_H

class Variant;

/**
 * A class that can be used to change a \c Variant's value incrementally with the mouse wheel.
 *
 * This is a development helper class that isn't meant to be included in releases.
 * This class might be helpful for example in fine tuning some physics related constant.
 * Pass some \c Variant that you are trying to find a suitable value for to this class.
 * Then adjust the value incrementally during run time with the mouse wheel and see the
 * effect of different values immediately on the screen. Off course the \c Variant that
 * is manipulated needs to be used somewhere in the code base so that changing its value
 * has a visible effect. This can be generally acchieved in two ways:
 *
 * 1. either pass a \c Variant to this class that is directly used by some other instance:
 *    \code VariantTuner::setTunableVariant(*myEntity->GetVar("pos2d"), CL_Vec2f(1, 0)); \endcode
 * 2. or listen to changes in a \c Variant if it's not directly used in a suitable place:
 * \code
 * // Define a listener method
 * void MyListener(Variant *v) {
 *     someVariableThatNeedsToBeModified->SetWeirdValue(v->GetFloat());
 * }
 *
 * // In some other part of the code
 * static Variant tunable(0.0f);
 * tunable.GetSigOnChanged()->connect(boost::bind(&MyListener, _1));
 * VariantTuner::setTunableVariant(tunable, 0.1f);
 * \endcode
 */
class VariantTuner
{
	VariantTuner();
public:
	/**
	 * Sets the \c Variant that is modified by this tuner.
	 *
	 * \a step sets the amount by which \a var is modified at a time.
	 * The types of \a var and \a step must match.
	 */
	static void setTunableVariant(Variant& var, const Variant& step);

	/**
	 * Stops modifiying any \c Variant that is currently set for modification.
	 */
	static void resetTunableVariant();
};

#endif // VARIANTTUNER_H
