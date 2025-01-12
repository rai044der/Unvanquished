#ifndef TURRET_COMPONENT_H_
#define TURRET_COMPONENT_H_

#include "../backend/CBSEBackend.h"
#include "../backend/CBSEComponents.h"

class TurretComponent: public TurretComponentBase {
	friend class MGTurretComponent;
	friend class RocketpodComponent;

	public:
		// ///////////////////// //
		// Autogenerated Members //
		// ///////////////////// //

		/**
		* @brief Default constructor of the TurretComponent.
		* @param entity The entity that owns the component instance.
		* @note This method is an interface for autogenerated code, do not modify its signature.
		*/
		TurretComponent(Entity& entity);

		/**
		* @brief Handle the PrepareNetCode message.
		* @note This method is an interface for autogenerated code, do not modify its signature.
		*/
		void HandlePrepareNetCode();

		// ///////////////////// //

		~TurretComponent();

	protected:
		void SetRange(float range);

		/**
		 * @brief The current entity target. Use TargetValid to check if it is still valid.
		 */
		Entity* GetTarget();

		/**
		 * @brief Whether there currently is a valid entity target.
		 */
		bool TargetValid();

		/**
		 * @brief Forget about the current entity target.
		 */
		void RemoveTarget();

		/**
		 * @brief Searches for a new entity target to be tracked (and shot at).
		 * @param CompareTargets A function that returns true if and only if the first target is the
		 *        better choice. For fairness with respect to the order of clients, the return value
		 *        should be chosen by a fair coin flip if both targets are to be considered equally
		 *        good.
		 * @return The target chosen or nullptr.
		 * @todo Allow TargetValid to be given as a parameter so specific turrets can use a
		 *       different validity check. Also fix the function to not only consider clients.
		 */
		Entity* FindEntityTarget(std::function<bool(Entity &, Entity &)> CompareTargets);

		/**
		 * @brief This moves the turret's head towards its target direction by an amount that
		 *        depends on the turret's speed and the last time this was called.
		 * @return Whether head points exactly in the target direction after the move.
		 */
		bool MoveHeadToTarget(int timeDelta);

		/**
		 * @brief Updates the turret's target direction to point towards its entity target.
		 */
		void TrackEntityTarget();

		/**
		 * @brief Resets the turret's target direction to its intial value.
		 */
		void ResetDirection();

		/**
		 * @brief Resets the height of the turret's target direction to its horizon.
		 */
		void ResetPitch();

		/**
		 * @brief Sets the height of the turret's target direction to the reachable minimum.
		 */
		void LowerPitch();

		/**
		 * @return Whether the current head position allows a shot to hit the entity target.
		 */
		bool TargetCanBeHit();

	private:
		/**
		 * @brief Decides on the turret's target direction after construction and after
		 *        subsequent calls to ResetDirection.
		 */
		void SetBaseDirection();

		/**
		 * @param newTarget Whether the entity is an existing target or the candidate for a new one.
		 * @return Whether the given target entity is a valid (new) target.
		 */
		bool TargetValid(Entity& target, bool newTarget);

		glm::vec3 TorsoAngles() const;
		glm::vec3 RelativeAnglesToAbsoluteAngles(const glm::vec3 relativeAngles) const;
		glm::vec3 AbsoluteAnglesToRelativeAngles(const glm::vec3 absoluteAngles) const;
		glm::vec3 DirectionToAbsoluteAngles(const glm::vec3 direction) const;
		glm::vec3 DirectionToRelativeAngles(const glm::vec3 direction) const;
		glm::vec3 AbsoluteAnglesToDirection(const glm::vec3 absoluteAngles) const;
		glm::vec3 RelativeAnglesToDirection(const glm::vec3 relativeAngles) const;

		/** An entity target that the turret can track. */
		GentityRef target;

		/** The attack range of the turret, used for checking whether a target can be hit. */
		float range;

		/** The turret's target direction after construction and a direction reset. */
		glm::vec3 baseDirection;

		/** The direction that the head will move to when asked to make a move. */
		glm::vec3 directionToTarget;

		/** The angle of the turret's head orientation relative to the turret's torso. */
		glm::vec3 relativeAimAngles;

		/** A timer used to forget about targets that were behind cover for a while. */
		int lastLineOfSightToTarget;
};

#endif // TURRET_COMPONENT_H_
