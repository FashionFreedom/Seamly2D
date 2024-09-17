/***************************************************************************
 **  @file   vcontainer.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   vcontainer.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vcontainer.h"

#include <limits.h>
#include <QVector>
#include <QtDebug>

#include "../ifc/exception/vexception.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vmisc/diagnostic.h"
#include "../vmisc/logging.h"
#include "../vmisc/vabstractapplication.h"
#include "variables/varcradius.h"
#include "variables/vcurveangle.h"
#include "variables/vcurvelength.h"
#include "variables/vcurveclength.h"
#include "variables/custom_variable.h"
#include "variables/vlineangle.h"
#include "variables/vlinelength.h"
#include "variables/measurement_variable.h"
#include "variables/vvariable.h"
#include "vtranslatevars.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vCon, "v.container")

QT_WARNING_POP

quint32 VContainer::_id = NULL_ID;
qreal VContainer::_size = 50;
qreal VContainer::_height = 176;
QSet<QString> VContainer::uniqueNames = QSet<QString>();

#ifdef Q_COMPILER_RVALUE_REFS
VContainer &VContainer::operator=(VContainer &&data) Q_DECL_NOTHROW
{ Swap(data); return *this; }
#endif

/**
 * @brief Swaps the contents of this VContainer with another.
 *
 * This method swaps the internal data pointer of this VContainer with that of another VContainer.
 * This operation is noexcept, meaning it guarantees not to throw any exceptions.
 *
 * @param data The other VContainer with which to swap contents.
 *
 * @details
 * - This method uses `std::swap` to exchange the data pointers of the two VContainer instances.
 * - It ensures that the swap operation is performed without throwing exceptions.
 */
void VContainer::Swap(VContainer &data) Q_DECL_NOTHROW
{ std::swap(d, data.d); }

/**
 * @brief Constructs a VContainer with the specified translation variables and pattern unit.
 *
 * This constructor initializes a VContainer instance by creating a new VContainerData object
 * with the provided translation variables and pattern unit.
 *
 * @param trVars A pointer to the translation variables.
 * @param patternUnit A pointer to the pattern unit.
 *
 * @details
 * - The constructor initializes the `d` member with a new VContainerData object.
 * - The `d` member is a shared data pointer, facilitating efficient data management.
 */
VContainer::VContainer(const VTranslateVars *trVars, const Unit *patternUnit)
    :d(new VContainerData(trVars, patternUnit))
{}

/**
 * @brief Assignment operator for VContainer.
 *
 * This operator assigns the contents of another VContainer to this instance.
 *
 * @param data The VContainer instance to assign from.
 * @return VContainer& A reference to this VContainer after assignment.
 *
 * @details
 * - The method first checks for self-assignment by comparing the address of the passed instance with this instance.
 * - If the addresses are the same, it returns a reference to this instance.
 * - Otherwise, it assigns the shared data pointer `d` from the passed instance to this instance.
 * - Finally, it returns a reference to this instance.
 */
VContainer &VContainer::operator =(const VContainer &data)
{
    if ( &data == this )
    {
        return *this;
    }
    d = data.d;
    return *this;
}

/**
 * @brief Copy constructor for VContainer.
 *
 * This constructor initializes a new VContainer instance as a copy of an existing VContainer.
 *
 * @param data The VContainer instance to copy from.
 *
 * @details
 * - The constructor initializes the shared data pointer `d` with the shared data pointer from the passed instance.
 * - This ensures that the new VContainer instance shares the same underlying data as the original.
 */
VContainer::VContainer(const VContainer &data)
    :d(data.d)
{}

/**
 * @brief Destructor for VContainer.
 *
 * This method handles the cleanup when a VContainer object is destroyed.
 *
 * @details
 * - The destructor calls `ClearGObjects()` to clear all geometric objects managed by the container.
 * - It then calls `ClearVariables()` to clear all variables managed by the container.
 * - This ensures that all resources are properly released when the VContainer is destroyed.
 */
VContainer::~VContainer()
{
    ClearGObjects();
    ClearVariables();
}

/**
 * @brief Retrieves a geometric object with the specified ID.
 *
 * This method retrieves a shared pointer to a geometric object of type VGObject from the container using the specified ID.
 *
 * @param id The ID of the geometric object to retrieve.
 * @return const QSharedPointer<VGObject> A shared pointer to the geometric object.
 *
 * @details
 * - The method delegates to the `GetObject` function, passing the internal hash map of geometric objects and the specified ID.
 * - If the object with the specified ID is found, it returns the shared pointer to the object.
 * - If the object with the specified ID is not found, an exception is thrown by the `GetObject` function.
 */
// cppcheck-suppress unusedFunction
const QSharedPointer<VGObject> VContainer::GetGObject(quint32 id)const
{
    return GetObject(d->gObjects, id);
}

/**
 * @brief Creates and retrieves a fake geometric object with the specified ID.
 *
 * This method creates a new VGObject with the given ID, wraps it in a shared pointer, and returns the shared pointer.
 *
 * @param id The ID to assign to the fake geometric object.
 * @return const QSharedPointer<VGObject> A shared pointer to the newly created fake geometric object.
 *
 * @details
 * - The method dynamically allocates a new VGObject and sets its ID to the specified value.
 * - It wraps the raw pointer to the new VGObject in a QSharedPointer to manage its lifetime automatically.
 * - It returns the shared pointer to the caller.
 */

const QSharedPointer<VGObject> VContainer::GetFakeGObject(quint32 id)
{
    VGObject *obj = new VGObject();
    obj->setId(id);
    QSharedPointer<VGObject> pointer(obj);
    return pointer;
}

/**
 * @brief Retrieves an object with the specified ID from a hash map.
 *
 * This template method retrieves an object of type val stored in the hash map
 * with the given key. If the object is not found, it throws an exception.
 *
 * @tparam key The type of the key used in the hash map.
 * @tparam val The type of the value stored in the hash map.
 * @param obj The hash map from which to retrieve the object.
 * @param id The key used to retrieve the object.
 * @return const val The object retrieved from the hash map.
 *
 * @throws VExceptionBadId if the object with the specified ID cannot be found.
 *
 * @details
 * - The method checks if the hash map contains the specified ID.
 * - If the ID is found, it returns the corresponding value.
 * - If the ID is not found, it throws a `VExceptionBadId` exception.
 */
template <typename key, typename val>
const val VContainer::GetObject(const QHash<key, val> &obj, key id) const
{
    if (obj.contains(id))
    {
        return obj.value(id);
    }
    else
    {
        throw VExceptionBadId(tr("Can't find object: "), id);
    }
}

/**
 * @brief Retrieves a piece with the specified ID.
 *
 * This method attempts to find and return a piece with the given ID from the container.
 * If the piece is not found, it throws an exception.
 *
 * @param id The ID of the piece to retrieve.
 * @return VPiece The piece retrieved from the container.
 *
 * @throws VExceptionBadId if the piece with the specified ID cannot be found.
 *
 * @details
 * - The method checks if the internal hash map of pieces contains the specified ID.
 * - If the ID is found, it returns the corresponding piece.
 * - If the ID is not found, it throws a `VExceptionBadId` exception.
 */
VPiece VContainer::GetPiece(quint32 id) const
{
    if (d->pieces->contains(id))
    {
        return d->pieces->value(id);
    }
    else
    {
        throw VExceptionBadId(tr("Can't find piece: "), id);
    }
}

/**
 * @brief Retrieves a piece path with the specified ID.
 *
 * This method attempts to find and return a piece path with the given ID from the container.
 * If the piece path is not found, it throws an exception.
 *
 * @param id The ID of the piece path to retrieve.
 * @return VPiecePath The piece path retrieved from the container.
 *
 * @throws VExceptionBadId if the piece path with the specified ID cannot be found.
 *
 * @details
 * - The method checks if the internal hash map of piece paths contains the specified ID.
 * - If the ID is found, it returns the corresponding piece path.
 * - If the ID is not found, it throws a `VExceptionBadId` exception.
 */
VPiecePath VContainer::GetPiecePath(quint32 id) const
{
    if (d->piecePaths->contains(id))
    {
        return d->piecePaths->value(id);
    }
    else
    {
        throw VExceptionBadId(tr("Can't find path: "), id);
    }
}

/**
 * @brief Adds a new geometric object to the container.
 *
 * This method adds a new geometric object to the container and returns its ID.
 *
 * @param obj A pointer to the new geometric object to add. The object must not be null.
 * @return quint32 The ID of the new object in the container.
 *
 * @details
 * - The method asserts that the provided object pointer is not null.
 * - It creates a QSharedPointer from the raw pointer to manage the object's lifetime.
 * - The name of the object is added to a set of unique names.
 * - It calls the `AddObject` method to add the shared pointer to the internal hash map of geometric objects.
 * - The method returns the ID assigned to the new object.
 */
quint32 VContainer::AddGObject(VGObject *obj)
{
    SCASSERT(obj != nullptr)
    QSharedPointer<VGObject> pointer(obj);
    uniqueNames.insert(obj->name());
    return AddObject(d->gObjects, pointer);
}

/**
 * @brief Adds a new piece to the container.
 *
 * This method adds a new piece to the container and returns its ID.
 *
 * @param piece The piece to add to the container.
 * @return quint32 The ID of the new piece in the container.
 *
 * @details
 * - The method generates a new unique ID for the piece by calling `getNextId()`.
 * - It inserts the piece into the internal hash map of pieces using the generated ID.
 * - The method returns the ID assigned to the new piece.
 */
quint32 VContainer::AddPiece(const VPiece &piece)
{
    const quint32 id = getNextId();
    d->pieces->insert(id, piece);
    return id;
}

/**
 * @brief Adds a new piece path to the container.
 *
 * This method adds a new piece path to the container and returns its ID.
 *
 * @param path The piece path to add to the container.
 * @return quint32 The ID of the new piece path in the container.
 *
 * @details
 * - The method generates a new unique ID for the piece path by calling `getNextId()`.
 * - It inserts the piece path into the internal hash map of piece paths using the generated ID.
 * - The method returns the ID assigned to the new piece path.
 */
quint32 VContainer::AddPiecePath(const VPiecePath &path)
{
    const quint32 id = getNextId();
    d->piecePaths->insert(id, path);
    return id;
}

/**
 * @brief Retrieves the current unique ID.
 *
 * This method returns the current value of the unique ID used for generating new IDs.
 *
 * @return quint32 The current unique ID.
 *
 * @details
 * - The method returns the value of the static member variable `_id`, which holds the current unique ID.
 */
quint32 VContainer::getId()
{
    return _id;
}

/**
 * @brief Generates and returns the next unique ID.
 *
 * This method increments the current unique ID and returns the new value.
 *
 * @return quint32 The next unique ID.
 *
 * @details
 * - The method first checks if the current ID has reached the maximum value (`UINT_MAX`).
 * - If the maximum value is reached, it logs a critical message indicating that the number of free IDs is exhausted.
 * - It increments the static member variable `_id` to generate the next unique ID.
 * - The method returns the new value of `_id`.
 *
 * @note Currently, the method simply increments the ID without reusing free IDs.
 */
quint32 VContainer::getNextId()
{
    //Because UINT_MAX is so big, it's impossible to run out of ids
    //We don't try to reuse free ids to avoid the complexity of tracking them
    if (_id == UINT_MAX)
    {
        qCritical() << (tr("Number of free id exhausted."));
    }
    _id++;
    return _id;
}

/**
 * @brief Updates the current unique ID to a new value if it is greater than the current value.
 *
 * This method sets the current unique ID to the specified new value if the new value is greater than the current value of `_id`.
 *
 * @param newId The new ID to set.
 *
 * @details
 * - The method checks if the specified `newId` is greater than the current value of `_id`.
 * - If the condition is met, it updates `_id` to the new value.
 * - If `newId` is not greater than the current value of `_id`, the method does nothing.
 */
void VContainer::UpdateId(quint32 newId)
{
    if (newId > _id)
    {
       _id = newId;
    }
}

/**
 * @brief Clears all data from the container.
 *
 * This method clears all data managed by the container, including pieces, piece paths, variables, and geometric objects.
 * It also resets the unique ID and clears the set of unique names.
 *
 * @details
 * - The method logs a debug message indicating that the container data is being cleared.
 * - It resets the unique ID to `NULL_ID`.
 * - It clears the internal hash maps of pieces and piece paths.
 * - It calls `ClearVariables()` to clear all variables managed by the container.
 * - It calls `ClearGObjects()` to clear all geometric objects managed by the container.
 * - It calls `ClearUniqueNames()` to clear the set of unique names.
 */
void VContainer::Clear()
{
    qCDebug(vCon, "Clearing container data.");
    _id = NULL_ID;

    d->pieces->clear();
    d->piecePaths->clear();
    ClearVariables();
    ClearGObjects();
    ClearUniqueNames();
}

/**
 * @brief Clears container data in preparation for a full parse.
 *
 * This method clears all relevant data from the container to prepare for a full parse operation.
 * It resets the unique ID and clears the set of unique names.
 *
 * @details
 * - The method logs a debug message indicating that the container data is being cleared for a full parse.
 * - It resets the unique ID to `NULL_ID`.
 * - It clears the internal hash maps of pieces and piece paths.
 * - It calls `ClearVariables()` multiple times to clear variables of different types:
 *   `Variable`, `LineAngle`, `LineLength`, `CurveLength`, `CurveCLength`, `ArcRadius`, and `CurveAngle`.
 * - It calls `ClearGObjects()` to clear all geometric objects managed by the container.
 * - It calls `ClearUniqueNames()` to clear the set of unique names.
 */
void VContainer::ClearForFullParse()
{
    qCDebug(vCon, "Clearing container data for full parse.");
    _id = NULL_ID;

    d->pieces->clear();
    d->piecePaths->clear();
    ClearVariables(VarType::Variable);
    ClearVariables(VarType::LineAngle);
    ClearVariables(VarType::LineLength);
    ClearVariables(VarType::CurveLength);
    ClearVariables(VarType::CurveCLength);
    ClearVariables(VarType::ArcRadius);
    ClearVariables(VarType::CurveAngle);
    ClearGObjects();
    ClearUniqueNames();
}

/**
 * @brief Clears all geometric objects from the container.
 *
 * This method removes all geometric objects managed by the container by clearing the internal hash map of geometric objects.
 *
 * @details
 * - The method clears the `gObjects` hash map, which stores all geometric objects in the container.
 * - After this operation, the container will no longer manage any geometric objects.
 */
void VContainer::ClearGObjects()
{
    d->gObjects.clear();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Clears all geometric objects marked for calculation from the container.
 *
 * This method removes all geometric objects that are marked with the mode `Draw::Calculation` from the container.
 *
 * @details
 * - The method first checks if the `gObjects` hash map is not empty.
 * - It iterates over the `gObjects` hash map to identify objects marked with `Draw::Calculation` mode.
 * - Identified objects are cleared and their keys are collected in a vector.
 * - After the iteration, the method removes the objects with the collected keys from the hash map.
 * - This two-step process ensures that the iterator is not invalidated during the removal of objects.
 */
void VContainer::ClearCalculationGObjects()
{
    if (not d->gObjects.isEmpty()) //-V807
    {
        QVector<quint32> keys;
        QHash<quint32, QSharedPointer<VGObject> >::iterator i;
        for (i = d->gObjects.begin(); i != d->gObjects.end(); ++i)
        {
            if (i.value()->getMode() == Draw::Calculation)
            {
                i.value().clear();
                keys.append(i.key());
            }
        }
        // We can't delete objects in previous loop it will destroy the iterator.
        if (not keys.isEmpty())
        {
            for (int i = 0; i < keys.size(); ++i)
            {
                d->gObjects.remove(keys.at(i));
            }
        }
    }
}

/**
 * @brief Clears variables of the specified type from the container.
 *
 * This method removes variables of the specified type from the container. If the type is `VarType::Unknown`, all variables are cleared.
 *
 * @param type The type of variables to clear.
 *
 * @details
 * - The method first checks if the `variables` hash map is not empty.
 * - If the specified type is `VarType::Unknown`, it clears the entire `variables` hash map.
 * - Otherwise, it iterates over the `variables` hash map to identify variables of the specified type.
 * - Identified variables' keys are collected in a vector.
 * - After the iteration, the method removes the variables with the collected keys from the hash map.
 * - This two-step process ensures that the iterator is not invalidated during the removal of variables.
 */
void VContainer::ClearVariables(const VarType &type)
{
    if (d->variables.size()>0) //-V807
    {
        if (type == VarType::Unknown)
        {
            d->variables.clear();
        }
        else
        {
            QVector<QString> keys;
            QHash<QString, QSharedPointer<VInternalVariable> >::iterator i;
            for (i = d->variables.begin(); i != d->variables.end(); ++i)
            {
                if (i.value()->GetType() == type)
                {
                    keys.append(i.key());
                }
            }

            for (int i = 0; i < keys.size(); ++i)
            {
                d->variables.remove(keys.at(i));
            }
        }
    }
}

/**
 * @brief AddLine add line to container
 * @param firstPointId id of first point of line
 * @param secondPointId id of second point of line
 */
void VContainer::AddLine(const quint32 &firstPointId, const quint32 &secondPointId)
{
    const QSharedPointer<VPointF> first = GeometricObject<VPointF>(firstPointId);
    const QSharedPointer<VPointF> second = GeometricObject<VPointF>(secondPointId);

    VLengthLine *length = new VLengthLine(first.data(), firstPointId, second.data(), secondPointId, *GetPatternUnit());
    AddVariable(length->GetName(), length);

    VLineAngle *angle = new VLineAngle(first.data(), firstPointId, second.data(), secondPointId);
    AddVariable(angle->GetName(), angle);
}

/**
 * @brief Adds a line defined by two points to the container.
 *
 * This method adds a line defined by the IDs of two points to the container. It creates length and angle variables for the line.
 *
 * @param firstPointId The ID of the first point defining the line.
 * @param secondPointId The ID of the second point defining the line.
 *
 * @details
 * - The method retrieves the geometric objects for the first and second points using their IDs.
 * - It creates a new `VLengthLine` object representing the length of the line between the two points.
 * - The `VLengthLine` object is added to the container as a variable.
 * - It also creates a new `VLineAngle` object representing the angle of the line between the two points.
 * - The `VLineAngle` object is added to the container as a variable.
 */
void VContainer::AddArc(const QSharedPointer<VAbstractCurve> &arc, const quint32 &id, const quint32 &parentId)
{
    AddCurve(arc, id, parentId);

    if (arc->getType() == GOType::Arc)
    {
        const QSharedPointer<VArc> casted = arc.staticCast<VArc>();

        VArcRadius *radius = new VArcRadius(id, parentId, casted.data(), *GetPatternUnit());
        AddVariable(radius->GetName(), radius);
    }
    else if (arc->getType() == GOType::EllipticalArc)
    {
        const QSharedPointer<VEllipticalArc> casted = arc.staticCast<VEllipticalArc>();

        VArcRadius *radius1 = new VArcRadius(id, parentId, casted.data(), 1, *GetPatternUnit());
        AddVariable(radius1->GetName(), radius1);

        VArcRadius *radius2 = new VArcRadius(id, parentId, casted.data(), 2, *GetPatternUnit());
        AddVariable(radius2->GetName(), radius2);
    }
}

/**
 * @brief Adds a curve to the container and creates related variables.
 *
 * This method adds a curve to the container and creates length and angle variables for the curve.
 * It supports specific types of curves, such as splines, cubic Bezier curves, arcs, and elliptical arcs.
 *
 * @param curve A shared pointer to the curve to add.
 * @param id The ID of the curve.
 * @param parentId The ID of the parent object, if any.
 *
 * @throws VException if the curve type is not supported.
 *
 * @details
 * - The method first retrieves the type of the curve.
 * - It checks if the curve type is one of the supported types: Spline, SplinePath, CubicBezier, CubicBezierPath, Arc, or EllipticalArc.
 * - If the curve type is not supported, it throws a `VException` with an appropriate message.
 * - It creates a new `VCurveLength` object representing the length of the curve and adds it as a variable.
 * - It creates a new `VCurveAngle` object representing the start angle of the curve and adds it as a variable.
 * - It creates a new `VCurveAngle` object representing the end angle of the curve and adds it as a variable.
 */
void VContainer::AddCurve(const QSharedPointer<VAbstractCurve> &curve, const quint32 &id, quint32 parentId)
{
    const GOType curveType = curve->getType();
    if (curveType != GOType::Spline      && curveType != GOType::SplinePath &&
        curveType != GOType::CubicBezier && curveType != GOType::CubicBezierPath &&
        curveType != GOType::Arc         && curveType != GOType::EllipticalArc)
    {
        throw VException(tr("Can't create a curve with type '%1'").arg(static_cast<int>(curveType)));
    }

    VCurveLength *length = new VCurveLength(id, parentId, curve.data(), *GetPatternUnit());
    AddVariable(length->GetName(), length);

    VCurveAngle *startAngle = new VCurveAngle(id, parentId, curve.data(), CurveAngle::StartAngle);
    AddVariable(startAngle->GetName(), startAngle);

    VCurveAngle *endAngle = new VCurveAngle(id, parentId, curve.data(), CurveAngle::EndAngle);
    AddVariable(endAngle->GetName(), endAngle);
}

/**
 * @brief Adds a spline to the container and creates related variables.
 *
 * This method adds a spline to the container and creates length variables for the control points of the spline.
 *
 * @param curve A shared pointer to the spline to add.
 * @param id The ID of the spline.
 * @param parentId The ID of the parent object, if any.
 *
 * @details
 * - The method first calls `AddCurve` to add the spline and create length and angle variables.
 * - It creates a new `VCurveCLength` object representing the length of the first control point (C1) and adds it as a variable.
 * - It creates a new `VCurveCLength` object representing the length of the second control point (C2) and adds it as a variable.
 */
void VContainer::AddSpline(const QSharedPointer<VAbstractBezier> &curve, quint32 id, quint32 parentId)
{
    AddCurve(curve, id, parentId);

    VCurveCLength *c1Length = new VCurveCLength(id, parentId, curve.data(), CurveCLength::C1, *GetPatternUnit());
    AddVariable(c1Length->GetName(), c1Length);

    VCurveCLength *c2Length = new VCurveCLength(id, parentId, curve.data(), CurveCLength::C2, *GetPatternUnit());
    AddVariable(c2Length->GetName(), c2Length);
}

/**
 * @brief Adds a cubic Bezier path with segments to the container and creates related variables.
 *
 * This method adds a cubic Bezier path to the container and creates length and angle variables for each segment of the path.
 *
 * @param curve A shared pointer to the cubic Bezier path to add.
 * @param id The ID of the cubic Bezier path.
 * @param parentId The ID of the parent object, if any.
 *
 * @details
 * - The method first calls `AddSpline` to add the cubic Bezier path and create length variables for the control points.
 * - It iterates over each segment of the cubic Bezier path.
 * - For each segment, it creates a `VCurveLength` object representing the length of the segment and adds it as a variable.
 * - It creates a `VCurveAngle` object representing the start angle of the segment and adds it as a variable.
 * - It creates a `VCurveAngle` object representing the end angle of the segment and adds it as a variable.
 * - It creates a `VCurveCLength` object representing the length of the first control point (C1) of the segment and adds it as a variable.
 * - It creates a `VCurveCLength` object representing the length of the second control point (C2) of the segment and adds it as a variable.
 */
void VContainer::AddCurveWithSegments(const QSharedPointer<VAbstractCubicBezierPath> &curve, const quint32 &id,
                                      quint32 parentId)
{
    AddSpline(curve, id, parentId);

    for (qint32 i = 1; i <= curve->CountSubSpl(); ++i)
    {
        const VSpline spl = curve->GetSpline(i);

        VCurveLength *length = new VCurveLength(id, parentId, curve->name(), spl, *GetPatternUnit(), i);
        AddVariable(length->GetName(), length);

        VCurveAngle *startAngle = new VCurveAngle(id, parentId, curve->name(), spl, CurveAngle::StartAngle, i);
        AddVariable(startAngle->GetName(), startAngle);

        VCurveAngle *endAngle = new VCurveAngle(id, parentId, curve->name(), spl, CurveAngle::EndAngle, i);
        AddVariable(endAngle->GetName(), endAngle);

        VCurveCLength *c1Length = new VCurveCLength(id, parentId, curve->name(), spl, CurveCLength::C1,
                                                    *GetPatternUnit(), i);
        AddVariable(c1Length->GetName(), c1Length);

        VCurveCLength *c2Length = new VCurveCLength(id, parentId, curve->name(), spl, CurveCLength::C2,
                                                    *GetPatternUnit(), i);
        AddVariable(c2Length->GetName(), c2Length);
    }
}

/**
 * @brief Removes a variable with the specified name from the container.
 *
 * This method removes a variable identified by its name from the container.
 *
 * @param name The name of the variable to remove.
 *
 * @details
 * - The method removes the variable with the specified name from the internal hash map of variables.
 * - If the variable with the specified name does not exist, the method does nothing.
 */
void VContainer::RemoveVariable(const QString &name)
{
    d->variables.remove(name);
}

/**
 * @brief Removes a piece with the specified ID from the container.
 *
 * This method removes a piece identified by its ID from the container.
 *
 * @param id The ID of the piece to remove.
 *
 * @details
 * - The method removes the piece with the specified ID from the internal hash map of pieces.
 * - If the piece with the specified ID does not exist, the method does nothing.
 */
void VContainer::RemovePiece(quint32 id)
{
    d->pieces->remove(id);
}

/**
 * @brief Adds an object to the specified hash map and assigns it a unique ID.
 *
 * This template method adds an object of type val to the specified hash map and assigns it a unique ID.
 *
 * @tparam key The type of the key used in the hash map.
 * @tparam val The type of the value stored in the hash map.
 * @param obj The hash map to which the object will be added.
 * @param value The object to add to the hash map.
 * @return quint32 The unique ID assigned to the added object.
 *
 * @details
 * - The method asserts that the provided object is not null.
 * - It generates a new unique ID for the object by calling `getNextId()`.
 * - It sets the ID of the object to the generated unique ID.
 * - It adds the object to the hash map with the generated unique ID as the key.
 * - The method returns the ID assigned to the added object.
 */
template <typename key, typename val>
quint32 VContainer::AddObject(QHash<key, val> &obj, val value)
{
    SCASSERT(value != nullptr)
    const quint32 id = getNextId();
    value->setId(id);
    obj[id] = value;
    return id;
}

/**
 * @brief Updates a piece with the specified ID in the container.
 *
 * This method updates the piece identified by its ID with the new piece data provided.
 *
 * @param id The ID of the piece to update.
 * @param piece The new data for the piece.
 *
 * @details
 * - The method asserts that the provided ID is not `NULL_ID`.
 * - It updates the piece with the specified ID in the internal hash map of pieces with the new data.
 * - It calls `UpdateId(id)` to ensure the unique ID tracker is updated if the provided ID is greater than the current tracker.
 */
void VContainer::UpdatePiece(quint32 id, const VPiece &piece)
{
    Q_ASSERT_X(id != NULL_ID, Q_FUNC_INFO, "id == 0"); //-V654 //-V712
    d->pieces->insert(id, piece);
    UpdateId(id);
}

/**
 * @brief Updates a piece path with the specified ID in the container.
 *
 * This method updates the piece path identified by its ID with the new path data provided.
 *
 * @param id The ID of the piece path to update.
 * @param path The new data for the piece path.
 *
 * @details
 * - The method asserts that the provided ID is not `NULL_ID`.
 * - It updates the piece path with the specified ID in the internal hash map of piece paths with the new data.
 * - It calls `UpdateId(id)` to ensure the unique ID tracker is updated if the provided ID is greater than the current tracker.
 */
void VContainer::UpdatePiecePath(quint32 id, const VPiecePath &path)
{
    Q_ASSERT_X(id != NULL_ID, Q_FUNC_INFO, "id == 0"); //-V654 //-V712
    d->piecePaths->insert(id, path);
    UpdateId(id);
}

/**
 * @brief Removes a custom variable with the specified name from the container.
 *
 * This method clears and removes a custom variable identified by its name from the container.
 *
 * @param name The name of the custom variable to remove.
 *
 * @details
 * - The method first clears the shared pointer associated with the specified variable name.
 * - It then removes the variable with the specified name from the internal hash map of variables.
 * - If the variable with the specified name does not exist, the method does nothing.
 */
void VContainer::removeCustomVariable(const QString &name)
{
    d->variables[name].clear();
    d->variables.remove(name);
}

/**
 * @brief Retrieves all measurement variables from the container.
 *
 * This method returns a map of all measurement variables stored in the container.
 *
 * @return QMap<QString, QSharedPointer<MeasurementVariable>> A map of measurement variables keyed by their names.
 *
 * @details
 * - The method calls `DataVar` with the type `MeasurementVariable` and the enumeration `VarType::Measurement`.
 * - It returns the map of measurement variables retrieved from the container.
 */
const QMap<QString, QSharedPointer<MeasurementVariable> > VContainer::DataMeasurements() const
{
    return DataVar<MeasurementVariable>(VarType::Measurement);
}

/**
 * @brief Retrieves all custom variables from the container.
 *
 * This method returns a map of all custom variables stored in the container.
 *
 * @return QMap<QString, QSharedPointer<CustomVariable>> A map of custom variables keyed by their names.
 *
 * @details
 * - The method calls `DataVar` with the type `CustomVariable` and the enumeration `VarType::Variable`.
 * - It returns the map of custom variables retrieved from the container.
 */
const QMap<QString, QSharedPointer<CustomVariable> > VContainer::variablesData() const
{
    return DataVar<CustomVariable>(VarType::Variable);
}

/**
 * @brief Retrieves all line length variables from the container.
 *
 * This method returns a map of all line length variables stored in the container.
 *
 * @return QMap<QString, QSharedPointer<VLengthLine>> A map of line length variables keyed by their names.
 *
 * @details
 * - The method calls `DataVar` with the type `VLengthLine` and the enumeration `VarType::LineLength`.
 * - It returns the map of line length variables retrieved from the container.
 */
const QMap<QString, QSharedPointer<VLengthLine> > VContainer::lineLengthsData() const
{
    return DataVar<VLengthLine>(VarType::LineLength);
}

/**
 * @brief Retrieves all curve length variables from the container.
 *
 * This method returns a map of all curve length variables stored in the container.
 *
 * @return QMap<QString, QSharedPointer<VCurveLength>> A map of curve length variables keyed by their names.
 *
 * @details
 * - The method calls `DataVar` with the type `VCurveLength` and the enumeration `VarType::CurveLength`.
 * - It returns the map of curve length variables retrieved from the container.
 */
const QMap<QString, QSharedPointer<VCurveLength> > VContainer::curveLengthsData() const
{
    return DataVar<VCurveLength>(VarType::CurveLength);
}

/**
 * @brief Retrieves all curve control point length variables from the container.
 *
 * This method returns a map of all curve control point length variables stored in the container.
 *
 * @return QMap<QString, QSharedPointer<VCurveCLength>> A map of curve control point length variables keyed by their names.
 *
 * @details
 * - The method calls `DataVar` with the type `VCurveCLength` and the enumeration `VarType::CurveCLength`.
 * - It returns the map of curve control point length variables retrieved from the container.
 */
const QMap<QString, QSharedPointer<VCurveCLength> > VContainer::controlPointLengthsData() const
{
    return DataVar<VCurveCLength>(VarType::CurveCLength);
}

/**
 * @brief Retrieves all line angle variables from the container.
 *
 * This method returns a map of all line angle variables stored in the container.
 *
 * @return QMap<QString, QSharedPointer<VLineAngle>> A map of line angle variables keyed by their names.
 *
 * @details
 * - The method calls `DataVar` with the type `VLineAngle` and the enumeration `VarType::LineAngle`.
 * - It returns the map of line angle variables retrieved from the container.
 */
const QMap<QString, QSharedPointer<VLineAngle> > VContainer::lineAnglesData() const
{
    return DataVar<VLineAngle>(VarType::LineAngle);
}

/**
 * @brief Retrieves all arc radius variables from the container.
 *
 * This method returns a map of all arc radius variables stored in the container.
 *
 * @return QMap<QString, QSharedPointer<VArcRadius>> A map of arc radius variables keyed by their names.
 *
 * @details
 * - The method calls `DataVar` with the type `VArcRadius` and the enumeration `VarType::ArcRadius`.
 * - It returns the map of arc radius variables retrieved from the container.
 */
const QMap<QString, QSharedPointer<VArcRadius> > VContainer::arcRadiusesData() const
{
    return DataVar<VArcRadius>(VarType::ArcRadius);
}

/**
 * @brief Retrieves all curve angle variables from the container.
 *
 * This method returns a map of all curve angle variables stored in the container.
 *
 * @return QMap<QString, QSharedPointer<VCurveAngle>> A map of curve angle variables keyed by their names.
 *
 * @details
 * - The method calls `DataVar` with the type `VCurveAngle` and the enumeration `VarType::CurveAngle`.
 * - It returns the map of curve angle variables retrieved from the container.
 */
const QMap<QString, QSharedPointer<VCurveAngle> > VContainer::curveAnglesData() const
{
    return DataVar<VCurveAngle>(VarType::CurveAngle);
}

/**
 * @brief Checks if a name is unique within the container.
 *
 * This method checks whether a given name is unique, meaning it is not present in the set of unique names or built-in functions.
 *
 * @param name The name to check for uniqueness.
 * @return bool True if the name is unique, false otherwise.
 *
 * @details
 * - The method returns true if the specified name is not found in the `uniqueNames` set and the `builInFunctions` set.
 * - If the name is found in either set, the method returns false.
 */
bool VContainer::IsUnique(const QString &name)
{
    return (!uniqueNames.contains(name) && !builInFunctions.contains(name));
}

/**
 * @brief Retrieves a list of all unique names within the container.
 *
 * This method returns a list of all unique names, including both built-in function names and custom unique names.
 *
 * @return QStringList A list of all unique names.
 *
 * @details
 * - The method initializes a QStringList with the names of built-in functions.
 * - It appends the values from the `uniqueNames` set to this list.
 * - The method returns the combined list of built-in function names and unique names.
 */
QStringList VContainer::AllUniqueNames()
{
    QStringList names = builInFunctions;
	names.append(uniqueNames.values());
    return names;
}

/**
 * @brief Retrieves the pattern unit associated with the container.
 *
 * This method returns a pointer to the pattern unit used by the container.
 *
 * @return const Unit* A pointer to the pattern unit.
 *
 * @details
 * - The method returns the `patternUnit` pointer stored in the shared data pointer `d`.
 */
const Unit *VContainer::GetPatternUnit() const
{
    return d->patternUnit;
}

/**
 * @brief Retrieves the translation variables associated with the container.
 *
 * This method returns a pointer to the translation variables used by the container.
 *
 * @return const VTranslateVars* A pointer to the translation variables.
 *
 * @details
 * - The method returns the `trVars` pointer stored in the shared data pointer `d`.
 */
const VTranslateVars *VContainer::getTranslateVariables() const
{
    return d->trVars;
}

/**
 * @brief Retrieves all variables of a specified type from the container.
 *
 * This template method returns a map of all variables of the specified type stored in the container.
 *
 * @tparam T The type of the variables to retrieve.
 * @param type The type of variables to retrieve, specified as a `VarType`.
 * @return QMap<QString, QSharedPointer<T>> A map of variables of the specified type, keyed by their names.
 *
 * @details
 * - The method initializes an empty QMap to store the variables.
 * - It iterates over the `variables` hash map, checking each variable's type.
 * - For each variable of the specified type, it retrieves the variable using the `getVariable` method.
 * - It inserts the variable into the map, converting the internal variable name to a user-friendly name using `VarToUser`.
 * - The method returns the map of variables of the specified type.
 */
template <typename T>
const QMap<QString, QSharedPointer<T> > VContainer::DataVar(const VarType &type) const
{
    QMap<QString, QSharedPointer<T> > map;
    //Sorting QHash by id
    QHash<QString, QSharedPointer<VInternalVariable> >::const_iterator i;
    for (i = d->variables.constBegin(); i != d->variables.constEnd(); ++i)
    {
        if (i.value()->GetType() == type)
        {
            QSharedPointer<T> var = getVariable<T>(i.key());
            map.insert(d->trVars->VarToUser(i.key()), var);
        }
    }
    return map;
}

/**
 * @brief Clears the set of unique names.
 *
 * This method removes all entries from the set of unique names managed by the container.
 *
 * @details
 * - The method clears the `uniqueNames` set, which stores the unique names of variables and objects in the container.
 * - After this operation, the container will no longer have any unique names stored.
 */
void VContainer::ClearUniqueNames()
{
    uniqueNames.clear();
}

/**
 * @brief Clears and resets the set of unique variable names.
 *
 * This method clears the set of unique names and then reinserts only those names that do not start with a '#' character.
 *
 * @details
 * - The method first retrieves all unique names as a QList.
 * - It clears the current set of unique names using `ClearUniqueNames()`.
 * - It iterates over the list of names, reinserting each name into the set of unique names unless it starts with a '#'.
 * - This effectively resets the set of unique names, excluding any names that begin with '#'.
 */
void VContainer::clearUniqueVariableNames()
{
	const QList<QString> list = uniqueNames.values();
    ClearUniqueNames();

    for(int i = 0; i < list.size(); ++i)
    {
        if (not list.at(i).startsWith('#'))
        {
            uniqueNames.insert(list.at(i));
        }
    }
}

/**
 * @brief Sets the multisize measurement size.
 *
 * This method sets the multisize measurement size to the specified value.
 *
 * @param size The new size to set.
 *
 * @details
 * - The method updates the static member variable `_size` with the provided value.
 */
void VContainer::setSize(qreal size)
{
    _size = size;
}

/**
 * @brief Sets the multisize measurement height.
 *
 * This method sets the multisize measurement height to the specified value.
 *
 * @param height The new height to set.
 *
 * @details
 * - The method updates the static member variable `_height` with the provided value.
 */
void VContainer::setHeight(qreal height)
{
    _height = height;
}

/**
 * @brief Returns the multisize measurement size.
 *
 * This method retrieves the current multisize measurement size.
 *
 * @return qreal The current multisize measurement size.
 *
 * @details
 * - The method returns the value of the static member variable `_size`.
 * - This size value is used in various calculations and operations.
 */
qreal VContainer::size()
{
    return _size;
}

/**
 * @brief Returns a pointer to the multisize measurement size.
 *
 * This method retrieves a pointer to the current the multisize measurement size.
 *
 * @return qreal* A pointer to the current the multisize measurement size.
 *
 * @details
 * - The method returns a pointer to the static member variable `_size`.
 */
qreal *VContainer::rsize()
{
    return &_size;
}

/**
 * @brief Returns the multisize measurement height.
 *
 * This method retrieves the current multisize measurement height.
 *
 * @return qreal The current multisize measurement height.
 *
 * @details
 * - The method returns the value of the static member variable `_height`.
 * - This height value is used in various calculations and operations.
 */
qreal VContainer::height()
{
    return _height;
}

/**
 * @brief Returns a pointer to the multisize measurement height.
 *
 * This method retrieves a pointer to the current multisize measurement height.
 *
 * @return qreal* A pointer to the current multisize measurement height.
 *
 * @details
 * - The method returns a pointer to the static member variable `_height`.
 */
qreal *VContainer::rheight()
{
    return &_height;
}

/**
 * @brief Returns a pointer to the the container of geometric objects.
 *
 * This method returns a pointer to the internal hash map of geometric objects managed by the container.
 *
 * @return const QHash<quint32, QSharedPointer<VGObject> >* A pointer to the hash map of geometric objects.
 *
 * @details
 * - The method returns a pointer to the `gObjects` hash map stored in the shared data pointer `d`.
 */
const QHash<quint32, QSharedPointer<VGObject> > *VContainer::DataGObjects() const
{
    return &d->gObjects;
}

/**
 * @brief Returns a pointer to the container of pieces.
 *
 * This method returns a pointer to the internal hash map of pieces managed by the container.
 *
 * @return const QHash<quint32, VPiece>* A pointer to the hash map of pieces.
 *
 * @details
 * - The method returns a pointer to the `pieces` hash map stored in the shared data pointer `d`.
 */
const QHash<quint32, VPiece> *VContainer::DataPieces() const
{
    return d->pieces.data();
}

/**
 * @brief Returns a pointer to the container of variables.
 *
 * This method returns a pointer to the internal hash map of variables managed by the container.
 *
 * @return const QHash<QString, QSharedPointer<VInternalVariable>>* A pointer to the hash map of variables.
 *
 * @details
 * - The method returns a pointer to the `variables` hash map stored in the shared data pointer `d`.
 */
const QHash<QString, QSharedPointer<VInternalVariable> > *VContainer::DataVariables() const
{
    return &d->variables;
}

/**
 * @brief Destructor for VContainerData.
 *
 * This method handles cleanup when a VContainerData object is destroyed.
 *
 * @details
 * - The destructor is defined but currently does not perform any specific actions.
 */
VContainerData::~VContainerData()
{}
