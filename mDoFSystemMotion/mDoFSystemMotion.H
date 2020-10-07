/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2019 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Class
    Foam::mDoFSystemMotion

Description
    Six degree of freedom motion for a rigid body.

    Angular momentum stored in body fixed reference frame.  Reference
    orientation of the body (where Q = I) must align with the cartesian axes
    such that the Inertia tensor is in principle component form.  Can add
    restraints (e.g. a spring) and constraints (e.g. motion may only be on a
    plane).

    The time-integrator for the motion is run-time selectable with options for
    symplectic (explicit), Crank-Nicolson and Newmark schemes.

SourceFiles
    mDoFSystemMotionI.H
    mDoFSystemMotion.C
    mDoFSystemMotionIO.C

\*---------------------------------------------------------------------------*/

#ifndef mDoFSystemMotion_H
#define mDoFSystemMotion_H

#include "mDoFSystemMotionState.H"
#include "pointField.H"
#include "mDoFSystemMotionRestraint.H"
#include "mDoFSystemMotionConstraint.H"
#include "Tuple2.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declarations
class mDoFSolver;

/*---------------------------------------------------------------------------*\
                      Class mDoFSystemMotion Declaration
\*---------------------------------------------------------------------------*/

class mDoFSystemMotion
{
    friend class mDoFSolver;

    // Private Data

        //- Motion state data object
        mDoFSystemMotionState motionState_;

        //- Motion state data object for previous time-step
        mDoFSystemMotionState motionState0_;

        //- Motion restraints
        PtrList<mDoFSystemMotionRestraint> restraints_;

        //- Motion constraints
        PtrList<mDoFSystemMotionConstraint> constraints_;

        //- Translational constraint tensor
        tensor tConstraints_;

        //- Rotational constraint tensor
        tensor rConstraints_;

        //- Centre of mass of initial state
        point initialCentreOfMass_;

        //- Centre of rotation of initial state
        point initialCentreOfRotation_;

        //- Orientation of initial state
        tensor initialQ_;

        //- Mass of the body
        scalar mass_;

        //- Moment of inertia of the body in reference configuration
        //  (Q = I)
        diagTensor momentOfInertia_;

        //- Acceleration relaxation coefficient
        scalar aRelax_;

        //- Acceleration damping coefficient (for steady-state simulations)
        scalar aDamp_;

        //- Switch to turn reporting of motion data on and off
        Switch report_;

        //- Motion solver
        autoPtr<mDoFSolver> solver_;


    // Private Member Functions

        //- Calculate the rotation tensor around the body reference
        //  frame x-axis by the given angle
        inline tensor rotationTensorX(scalar deltaT) const;

        //- Calculate the rotation tensor around the body reference
        //  frame y-axis by the given angle
        inline tensor rotationTensorY(scalar deltaT) const;

        //- Calculate the rotation tensor around the body reference
        //  frame z-axis by the given angle
        inline tensor rotationTensorZ(scalar deltaT) const;

        //- Apply rotation tensors to Q0 for the given torque (pi) and deltaT
        //  and return the rotated Q and pi as a tuple
        inline Tuple2<tensor, vector> rotate
        (
            const tensor& Q0,
            const vector& pi,
            const scalar deltaT
        ) const;

        //- Apply the restraints to the object
        void applyRestraints();

        //- Update and relax accelerations from the force and torque
        void updateAcceleration(const vector& fGlobal, const vector& tauGlobal);


        // Access functions retained as private because of the risk of
        // confusion over what is a body local frame vector and what is global

        // Access

            //- Return the restraints
            inline const PtrList<mDoFSystemMotionRestraint>&
                restraints() const;

            //- Return the constraints
            inline const PtrList<mDoFSystemMotionConstraint>&
                constraints() const;

            //- Return the initial centre of rotation
            inline const point& initialCentreOfRotation() const;

            //- Return the initial orientation
            inline const tensor& initialQ() const;

            //- Return the orientation
            inline const tensor& Q() const;

            //- Return the current acceleration
            inline const vector& a() const;

            //- Return the current angular momentum
            inline const vector& pi() const;

            //- Return the current torque
            inline const vector& tau() const;


        // Edit

            //- Return the centre of rotation
            inline point& initialCentreOfRotation();

            //- Return initial orientation
            inline tensor& initialQ();

            //- Return non-const access to the orientation
            inline tensor& Q();

            //- Return non-const access to vector
            inline vector& v();

            //- Return non-const access to acceleration
            inline vector& a();

            //- Return non-const access to angular momentum
            inline vector& pi();

            //- Return non-const access to torque
            inline vector& tau();


public:

    //- Runtime type information
    TypeName("mDoFSystemMotion");


    // Constructors

        //- Construct null
        mDoFSystemMotion();

        //- Construct from constant and state dictionaries
        mDoFSystemMotion
        (
            const dictionary& dict,
            const dictionary& stateDict
        );

        //- Copy constructor
        mDoFSystemMotion(const mDoFSystemMotion&);


    //- Destructor
    virtual ~mDoFSystemMotion();


    // Member Functions

        // Access

            //- Return the mass
            inline scalar mass() const;

            //- Return the inertia tensor
            inline const diagTensor& momentOfInertia() const;

            //- Return the motion state
            inline const mDoFSystemMotionState& state() const;

            //- Return the current centre of rotation
            inline const point& centreOfRotation() const;

            //- Return the initial centre of mass
            inline const point& initialCentreOfMass() const;

            //- Return the current centre of mass
            inline point centreOfMass() const;

            //- Return the orientation tensor, Q.
            //  globalVector = Q & bodyLocalVector
            //  bodyLocalVector = Q.T() & globalVector
            inline const tensor& orientation() const;

            //- Return the angular velocity in the global frame
            inline vector omega() const;

            //- Return the current velocity
            inline const vector& v() const;

            inline vector momentArm() const;

            //- Return the report Switch
            inline bool report() const;


        // Edit

            //- Store the motion state at the beginning of the time-step
            inline void newTime();

            //- Return non-const access to the centre of rotation
            inline point& centreOfRotation();


        // Constraints and Restraints

            //- Add restraints to the motion, public to allow external
            //  addition of restraints after construction
            void addRestraints(const dictionary& dict);

            //- Add restraints to the motion, public to allow external
            //  addition of restraints after construction
            void addConstraints(const dictionary& dict);


        // Update state

            //- Symplectic integration of velocities, orientation and position.
            //  Changes to Crank-Nicolson integration for subsequent iterations.
            void update
            (
                bool firstIter,
                const vector& fGlobal,
                const vector& tauGlobal,
                scalar deltaT,
                scalar deltaT0
            );

            //- Report the status of the motion
            void status() const;


        // Transformations

            //- Return the velocity of a position
            inline point velocity(const point& pt) const;

            //- Transform the given initial state point by the current motion
            //  state
            inline point transform(const point& initialPoints) const;

            //- Transform the given initial state pointField by the current
            //  motion state
            tmp<pointField> transform(const pointField& initialPoints) const;

            //- Transform the given initial state pointField by the current
            //  motion state scaled by the given scale
            tmp<pointField> transform
            (
                const pointField& initialPoints,
                const scalarField& scale
            ) const;


        //- Write
        void write(Ostream&) const;

        //- Read coefficients dictionary and update system parameters,
        //  constraints and restraints but not the current state
        bool read(const dictionary& dict);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "mDoFSystemMotionI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //