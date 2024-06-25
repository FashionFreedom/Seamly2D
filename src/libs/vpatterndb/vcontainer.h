/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
 *                                                                         *
 ***************************************************************************
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
 **************************************************************************

 ************************************************************************
 **
 **  @file   vcontainer.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
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

#ifndef VCONTAINER_H
#define VCONTAINER_H

#include <qcompilerdetection.h>
#include <QCoreApplication>
#include <QHash>
#include <QMap>
#include <QMessageLogger>
#include <QSet>
#include <QSharedPointer>
#include <QSharedData>
#include <QSharedDataPointer>
#include <QString>
#include <QStringList>
#include <QTypeInfo>
#include <QtGlobal>
#include <new>

#include "../vmisc/def.h"
#include "../ifc/exception/vexceptionbadid.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgobject.h"
#include "../vmisc/diagnostic.h"
#include "variables.h"
#include "variables/vinternalvariable.h"
#include "vpiece.h"
#include "vpiecepath.h"
#include "vtranslatevars.h"

class VEllipticalArc;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_INTEL(2021)
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

/**
 * @class VContainerData
 * @brief A data container class that uses Qt's implicit sharing mechanism.
 * 
 * The VContainerData class is designed to efficiently manage shared data
 * using Qt's implicit sharing (copy-on-write) mechanism. It contains
 * several hash maps and shared pointers to handle various elements.
 * 
 * @details
 * The class inherits from QSharedData to leverage Qt's implicit sharing.
 * It has two constructors: one for initializing its member variables and 
 * another for copying the contents from another instance. The class manages 
 * the following members:
 * 
 * - `gObjects`: A hash map mapping a quint32 to a shared pointer of VGObject.
 * - `variables`: A hash map mapping a QString to a shared pointer of VInternalVariable.
 * - `pieces`: A shared pointer to a hash map mapping a quint32 to VPiece.
 * - `piecePaths`: A shared pointer to a hash map mapping a quint32 to VPiecePath.
 * - `trVars`: A pointer to a VTranslateVars object.
 * - `patternUnit`: A pointer to a Unit object.
 * 
 * The class provides a constructor for initializing these members and a 
 * copy constructor to create copies of the data container.
 */

class VContainerData : public QSharedData //-V690
{
public:

    VContainerData(const VTranslateVars *trVars, const Unit *patternUnit)
        : gObjects(QHash<quint32, QSharedPointer<VGObject> >()),
          variables(QHash<QString, QSharedPointer<VInternalVariable> > ()),
          pieces(QSharedPointer<QHash<quint32, VPiece>>(new QHash<quint32, VPiece>())),
          piecePaths(QSharedPointer<QHash<quint32, VPiecePath>>(new QHash<quint32, VPiecePath>())),
          trVars(trVars),
          patternUnit(patternUnit)
    {}

    VContainerData(const VContainerData &data)
        : QSharedData(data),
          gObjects(data.gObjects),
          variables(data.variables),
          pieces(data.pieces),
          piecePaths(data.piecePaths),
          trVars(data.trVars),
          patternUnit(data.patternUnit)
    {}

    virtual ~VContainerData();

    /**
     * @brief gObjects graphicals objects of pattern.
     */
    QHash<quint32, QSharedPointer<VGObject> > gObjects;

    /**
     * @brief variables container for measurements, custom variables, lines lengths, lines angles, arcs lengths, curve lengths
     */
    QHash<QString, QSharedPointer<VInternalVariable>> variables;

    QSharedPointer<QHash<quint32, VPiece>> pieces;
    QSharedPointer<QHash<quint32, VPiecePath>> piecePaths;

    const VTranslateVars *trVars;
    const Unit *patternUnit;

private:
    VContainerData &operator=(const VContainerData &) Q_DECL_EQ_DELETE;
};

QT_WARNING_POP

/**
 * @class VContainer
 * @brief Manages a collection of geometric objects, pieces, paths, and variables.
 *
 * The VContainer class provides an interface to manage and manipulate various elements
 * such as geometric objects, pieces, paths, and variables. It supports operations like
 * adding, updating, and removing these elements, as well as querying for specific data.
 *
 * @details
 * The class uses implicit sharing via QSharedDataPointer to manage shared data efficiently.
 * It includes functions for adding, updating, and removing geometric objects, pieces, paths,
 * and variables. Additionally, the class provides static methods for handling unique IDs
 * and sizes, as well as static data members for maintaining state across instances.
 *
 * The following key functionalities are supported:
 * - Adding, updating, and retrieving geometric objects (`VGObject`).
 * - Adding, updating, and retrieving pieces (`VPiece`).
 * - Adding, updating, and retrieving piece paths (`VPiecePath`).
 * - Managing variables, including custom and internal variables.
 * - Clearing various categories of managed objects and variables.
 * - Maintaining unique names for variables.
 * - Static methods for setting and getting size and height parameters.
 *
 * The class also supports move semantics and provides template methods for handling various types.
 *
 * @note The class makes extensive use of Qt containers and shared pointers to manage its data.
 *
 * @public
 * - VContainer(const VTranslateVars *trVars, const Unit *patternUnit);
 * - VContainer(const VContainer &data);
 * - ~VContainer();
 * - VContainer &operator=(const VContainer &data);
 * - VContainer &operator=(VContainer &&data) Q_DECL_NOTHROW;
 * - void Swap(VContainer &data) Q_DECL_NOTHROW;
 * - template <typename T> const QSharedPointer<T> GeometricObject(const quint32 &id) const;
 * - const QSharedPointer<VGObject> GetGObject(quint32 id) const;
 * - static const QSharedPointer<VGObject> GetFakeGObject(quint32 id);
 * - VPiece GetPiece(quint32 id) const;
 * - VPiecePath GetPiecePath(quint32 id) const;
 * - template <typename T> QSharedPointer<T> getVariable(QString name) const;
 * - static quint32 getId();
 * - static quint32 getNextId();
 * - static void UpdateId(quint32 newId);
 * - quint32 AddGObject(VGObject *obj);
 * - quint32 AddPiece(const VPiece &piece);
 * - quint32 AddPiecePath(const VPiecePath &path);
 * - void AddLine(const quint32 &firstPointId, const quint32 &secondPointId);
 * - void AddArc(const QSharedPointer<VAbstractCurve> &arc, const quint32 &id, const quint32 &parentId = NULL_ID);
 * - void AddSpline(const QSharedPointer<VAbstractBezier> &curve, quint32 id, quint32 parentId = NULL_ID);
 * - void AddCurveWithSegments(const QSharedPointer<VAbstractCubicBezierPath> &curve, const quint32 &id, quint32 parentId = NULL_ID);
 * - template <typename T> void AddVariable(const QString& name, T *var);
 * - template <typename T> void AddVariable(const QString& name, const QSharedPointer<T> &var);
 * - void RemoveVariable(const QString& name);
 * - void RemovePiece(quint32 id);
 * - template <class T> void UpdateGObject(quint32 id, T* obj);
 * - template <class T> void UpdateGObject(quint32 id, const QSharedPointer<T> &obj);
 * - void UpdatePiece(quint32 id, const VPiece &piece);
 * - void UpdatePiecePath(quint32 id, const VPiecePath &path);
 * - void Clear();
 * - void ClearForFullParse();
 * - void ClearGObjects();
 * - void ClearCalculationGObjects();
 * - void ClearVariables(const VarType &type = VarType::Unknown);
 * - static void ClearUniqueNames();
 * - static void clearUniqueVariableNames();
 * - static void setSize(qreal size);
 * - static void setHeight(qreal height);
 * - static qreal size();
 * - static qreal *rsize();
 * - static qreal height();
 * - static qreal *rheight();
 * - void removeCustomVariable(const QString& name);
 * - const QHash<quint32, QSharedPointer<VGObject> > *DataGObjects() const;
 * - const QHash<quint32, VPiece> *DataPieces() const;
 * - const QHash<QString, QSharedPointer<VInternalVariable>> *DataVariables() const;
 * - const QMap<QString, QSharedPointer<MeasurementVariable> > DataMeasurements() const;
 * - const QMap<QString, QSharedPointer<CustomVariable> > variablesData() const;
 * - const QMap<QString, QSharedPointer<VLengthLine> > lineLengthsData() const;
 * - const QMap<QString, QSharedPointer<VCurveLength> > curveLengthsData() const;
 * - const QMap<QString, QSharedPointer<VCurveCLength> > controlPointLengthsData() const;
 * - const QMap<QString, QSharedPointer<VLineAngle> > lineAnglesData() const;
 * - const QMap<QString, QSharedPointer<VArcRadius> > arcRadiusesData() const;
 * - const QMap<QString, QSharedPointer<VCurveAngle> > curveAnglesData() const;
 * - static bool IsUnique(const QString &name);
 * - static QStringList AllUniqueNames();
 * - const Unit *GetPatternUnit() const;
 * - const VTranslateVars *getTranslateVariables() const;
 *
 * @private
 * - static quint32 _id; New object will have value +1. For empty class equal 0.
 * - static qreal _size;
 * - static qreal _height;
 * - static QSet<QString> uniqueNames;
 * - QSharedDataPointer<VContainerData> d;
 * - void AddCurve(const QSharedPointer<VAbstractCurve> &curve, const quint32 &id, quint32 parentId = NULL_ID);
 * - template <class T> uint qHash(const QSharedPointer<T> &p);
 * - template <typename key, typename val> const val GetObject(const QHash<key, val> &obj, key id) const;
 * - template <typename T> void UpdateObject(const quint32 &id, const QSharedPointer<T> &point);
 * - template <typename key, typename val> static quint32 AddObject(QHash<key, val> &obj, val value);
 * - template <typename T> const QMap<QString, QSharedPointer<T> > DataVar(const VarType &type) const;
 */
class VContainer
{
    Q_DECLARE_TR_FUNCTIONS(VContainer)
public:
    VContainer(const VTranslateVars *trVars, const Unit *patternUnit);
    VContainer(const VContainer &data);
    ~VContainer();

    VContainer &operator=(const VContainer &data);
#ifdef Q_COMPILER_RVALUE_REFS
	VContainer &operator=(VContainer &&data) Q_DECL_NOTHROW;
#endif

	void Swap(VContainer &data) Q_DECL_NOTHROW;

    template <typename T>
    const QSharedPointer<T> GeometricObject(const quint32 &id) const;
    const QSharedPointer<VGObject> GetGObject(quint32 id) const;
    static const QSharedPointer<VGObject> GetFakeGObject(quint32 id);
    VPiece             GetPiece(quint32 id) const;
    VPiecePath         GetPiecePath(quint32 id) const;
    template <typename T>
    QSharedPointer<T>  getVariable(QString name) const;
    static quint32     getId();
    static quint32     getNextId();
    static void        UpdateId(quint32 newId);

    quint32            AddGObject(VGObject *obj);
    quint32            AddPiece(const VPiece &piece);
    quint32            AddPiecePath(const VPiecePath &path);
    void               AddLine(const quint32 &firstPointId, const quint32 &secondPointId);
    void               AddArc(const QSharedPointer<VAbstractCurve> &arc, const quint32 &id,
                              const quint32 &parentId = NULL_ID);
    void               AddSpline(const QSharedPointer<VAbstractBezier> &curve, quint32 id, quint32 parentId = NULL_ID);
    void               AddCurveWithSegments(const QSharedPointer<VAbstractCubicBezierPath> &curve, const quint32 &id,
                                            quint32 parentId = NULL_ID);

    template <typename T>
    void               AddVariable(const QString& name, T *var);
    template <typename T>
    void               AddVariable(const QString& name, const QSharedPointer<T> &var);
    void               RemoveVariable(const QString& name);
    void               RemovePiece(quint32 id);

    template <class T>
    void               UpdateGObject(quint32 id, T* obj);
    template <class T>
    void               UpdateGObject(quint32 id, const QSharedPointer<T> &obj);

    void               UpdatePiece(quint32 id, const VPiece &piece);
    void               UpdatePiecePath(quint32 id, const VPiecePath &path);

    void               Clear();
    void               ClearForFullParse();
    void               ClearGObjects();
    void               ClearCalculationGObjects();
    void               ClearVariables(const VarType &type = VarType::Unknown);
    static void        ClearUniqueNames();
    static void        clearUniqueVariableNames();

    static void        setSize(qreal size);
    static void        setHeight(qreal height);
    static qreal       size();
    static qreal      *rsize();
    static qreal       height();
    static qreal      *rheight();

    void               removeCustomVariable(const QString& name);

    const QHash<quint32, QSharedPointer<VGObject> >         *DataGObjects() const;
    const QHash<quint32, VPiece>                            *DataPieces() const;
    const QHash<QString, QSharedPointer<VInternalVariable>> *DataVariables() const;

    const QMap<QString, QSharedPointer<MeasurementVariable> >  DataMeasurements() const;
    const QMap<QString, QSharedPointer<CustomVariable> >    variablesData() const;
    const QMap<QString, QSharedPointer<VLengthLine> >   lineLengthsData() const;
    const QMap<QString, QSharedPointer<VCurveLength> >  curveLengthsData() const;
    const QMap<QString, QSharedPointer<VCurveCLength> > controlPointLengthsData() const;
    const QMap<QString, QSharedPointer<VLineAngle> >    lineAnglesData() const;
    const QMap<QString, QSharedPointer<VArcRadius> >    arcRadiusesData() const;
    const QMap<QString, QSharedPointer<VCurveAngle> >   curveAnglesData() const;

    static bool        IsUnique(const QString &name);
    static QStringList AllUniqueNames();

    const Unit *GetPatternUnit() const;
    const VTranslateVars *getTranslateVariables() const;

private:
    static quint32 _id;
    static qreal   _size;
    static qreal   _height;
    static QSet<QString> uniqueNames;

    QSharedDataPointer<VContainerData> d;

    void AddCurve(const QSharedPointer<VAbstractCurve> &curve, const quint32 &id, quint32 parentId = NULL_ID);

    template <class T>
    uint qHash( const QSharedPointer<T> &p );

    template <typename key, typename val>
    // cppcheck-suppress functionStatic
    const val GetObject(const QHash<key, val> &obj, key id) const;

    template <typename T>
    void UpdateObject(const quint32 &id, const QSharedPointer<T> &point);

    template <typename key, typename val>
    static quint32 AddObject(QHash<key, val> &obj, val value);

    template <typename T>
    const QMap<QString, QSharedPointer<T> > DataVar(const VarType &type) const;
};

Q_DECLARE_TYPEINFO(VContainer, Q_MOVABLE_TYPE);

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Retrieves a shared pointer to a geometric object of type T with the specified ID.
 * 
 * This template method attempts to find and return a geometric object of type T stored in the container
 * with the given ID. It performs a dynamic cast to ensure the object is of the expected type.
 * 
 * @tparam T The expected type of the geometric object to retrieve.
 * @param id The ID of the geometric object to retrieve.
 * @return const QSharedPointer<T> A shared pointer to the geometric object of type T.
 * 
 * @throws VExceptionBadId if the object ID is NULL_ID, if the object cannot be found, or if the object cannot be cast to the expected type.
 * 
 * @details
 * - The method first checks if the provided ID is `NULL_ID` and throws a `VExceptionBadId` exception if it is.
 * - It then attempts to find the geometric object with the specified ID in the container.
 * - If the object is found, it attempts to cast it to the expected type T using `qSharedPointerDynamicCast`.
 * - If the cast is successful, it returns the shared pointer to the geometric object.
 * - If the cast fails due to a bad allocation, it throws a `VExceptionBadId` exception with an appropriate message.
 * - If the object with the specified ID does not exist, it throws a `VExceptionBadId` exception.
 */
template <typename T>
const QSharedPointer<T> VContainer::GeometricObject(const quint32 &id) const
{

    if (id == NULL_ID)
    {
        throw VExceptionBadId(tr("Can't find object"), id);
    }

    QSharedPointer<VGObject> gObj = QSharedPointer<VGObject>();
    if (d->gObjects.contains(id))
    {
        gObj = d->gObjects.value(id);
    }
    else
    {
        throw VExceptionBadId(tr("Can't find object Id: "), id);
    }
    try
    {
        QSharedPointer<T> obj = qSharedPointerDynamicCast<T>(gObj);
        SCASSERT(obj.isNull() == false)
        return obj;
    }
    catch (const std::bad_alloc &)
    {
        throw VExceptionBadId(tr("Can't cast object"), id);
    }
}

/**
 * @brief Retrieves a shared pointer to a variable of type T with the specified name.
 * 
 * This template method attempts to find and return a variable of type T stored in the container
 * with the given name. It performs a dynamic cast to ensure the variable is of the expected type.
 * 
 * @tparam T The expected type of the variable to retrieve.
 * @param name The name of the variable to retrieve.
 * @return QSharedPointer<T> A shared pointer to the variable of type T.
 * 
 * @throws VExceptionBadId if the variable cannot be found or cannot be cast to the expected type.
 * 
 * @note The method includes assertions to check that the name is not empty and that the cast is successful.
 * 
 * @details
 * - The method first asserts that the provided name is not empty.
 * - It checks if the variable with the specified name exists in the container.
 * - If the variable exists, it attempts to cast it to the expected type T using `qSharedPointerDynamicCast`.
 * - If the cast is successful, it returns the shared pointer to the variable.
 * - If the cast fails due to a bad allocation, it throws a `VExceptionBadId` exception with an appropriate message.
 * - If the variable with the specified name does not exist, it throws a `VExceptionBadId` exception.
 */
template <typename T>
QSharedPointer<T> VContainer::getVariable(QString name) const
{
    SCASSERT(name.isEmpty()==false)
    if (d->variables.contains(name))
    {
        try
        {
            QSharedPointer<T> value = qSharedPointerDynamicCast<T>(d->variables.value(name));
            SCASSERT(value.isNull() == false)
            return value;
        }
        catch (const std::bad_alloc &)
        {
            throw VExceptionBadId(tr("Can't cast object"), name);
        }
    }
    else
    {
        throw VExceptionBadId(tr("Can't find object"), name);
    }
}

/**
 * @brief Adds a variable with the specified name and pointer to the container.
 * 
 * This template method adds a variable of type T to the container, using a raw pointer.
 * It wraps the raw pointer in a QSharedPointer and then delegates to another AddVariable method.
 * 
 * @tparam T The type of the variable to add.
 * @param name The name of the variable to add.
 * @param var A raw pointer to the variable to add.
 * 
 * @details
 * - The method creates a QSharedPointer from the raw pointer.
 * - It calls another AddVariable method to add the shared pointer to the container.
 */
template <typename T>
void VContainer::AddVariable(const QString& name, T *var)
{
    AddVariable(name, QSharedPointer<T>(var));
}

/**
 * @brief Adds or updates a variable with the specified name and shared pointer to the container.
 * 
 * This template method adds a variable of type T to the container using a shared pointer.
 * If a variable with the same name already exists and is of the same type, it updates the existing variable.
 * If the types do not match, it throws an exception.
 * 
 * @tparam T The type of the variable to add.
 * @param name The name of the variable to add.
 * @param var A shared pointer to the variable to add.
 * 
 * @throws VExceptionBadId if the variable cannot be cast to the expected type or if there is a type mismatch.
 * 
 * @details
 * - The method first checks if a variable with the specified name already exists in the container.
 * - If it exists and is of the same type, it updates the existing variable with the new value.
 * - If the existing variable cannot be cast to the expected type, it throws a `VExceptionBadId` exception.
 * - If the types do not match, it throws a `VExceptionBadId` exception.
 * - If the variable does not already exist, it adds the new variable to the container.
 * - The method also adds the variable name to a set of unique names.
 */
template <typename T>
void VContainer::AddVariable(const QString& name, const QSharedPointer<T> &var)
{
    if (d->variables.contains(name))
    {
        if (d->variables.value(name)->GetType() == var->GetType())
        {
            QSharedPointer<T> v = qSharedPointerDynamicCast<T>(d->variables.value(name));
            if (v.isNull())
            {
                throw VExceptionBadId(tr("Can't cast object."), name);
            }
            *v = *var;
        }
        else
        {
            throw VExceptionBadId(tr("Can't find object. Type mismatch."), name);
        }
    }
    else
    {
        d->variables.insert(name, var);
    }

    uniqueNames.insert(name);
}

/**
 * @brief Calculates the hash value for a shared pointer to type T.
 * 
 * This template method calculates the hash value for a shared pointer to type T.
 * It delegates to the qHash function for the raw pointer stored in the shared pointer.
 * 
 * @tparam T The type of the shared pointer.
 * @param p The shared pointer for which to calculate the hash value.
 * @return uint The hash value calculated for the shared pointer.
 * 
 * @details
 * - The method extracts the raw pointer from the shared pointer using the data() method.
 * - It then delegates to the qHash function for the raw pointer to calculate the hash value.
 * - The hash value is returned as a uint.
 */
template <class T>
uint VContainer::qHash( const QSharedPointer<T> &p )
{
    return qHash( p.data() );
}

/**
 * @brief Updates a geometric object with the specified ID using a raw pointer.
 * 
 * This template method updates a geometric object of type T in the container using a raw pointer.
 * It wraps the raw pointer in a QSharedPointer and then delegates to another UpdateGObject method.
 * 
 * @tparam T The type of the geometric object to update.
 * @param id The ID of the geometric object to update.
 * @param obj A raw pointer to the geometric object to update.
 * 
 * @details
 * - The method asserts that the raw pointer is not null.
 * - It creates a QSharedPointer from the raw pointer.
 * - It calls another UpdateGObject method to update the geometric object in the container.
 */
 template <class T>
void VContainer::UpdateGObject(quint32 id, T* obj)
{
    SCASSERT(obj != nullptr)
    UpdateGObject(id, QSharedPointer<T>(obj));
}

template <class T>
void VContainer::UpdateGObject(quint32 id, const QSharedPointer<T> &obj)
{
    SCASSERT(not obj.isNull())
    UpdateObject(id, obj);
    uniqueNames.insert(obj->name());
}
/**
 * @brief Updates a geometric object with the specified ID using a shared pointer.
 * 
 * This template method updates a geometric object of type T in the container using a shared pointer.
 * It ensures the shared pointer is not null and delegates to another method to perform the update.
 * 
 * @tparam T The type of the geometric object to update.
 * @param id The ID of the geometric object to update.
 * @param obj A shared pointer to the geometric object to update.
 * 
 * @details
 * - The method asserts that the shared pointer is not null.
 * - It calls the UpdateObject method to update the geometric object in the container with the specified ID.
 * - It inserts the name of the object into a set of unique names.
 */
template <typename T>
void VContainer::UpdateObject(const quint32 &id, const QSharedPointer<T> &point)
{
    Q_ASSERT_X(id != NULL_ID, Q_FUNC_INFO, "id == 0"); //-V654 //-V712
    SCASSERT(point.isNull() == false)
    point->setId(id);
    if (d->gObjects.contains(id))
    {
        QSharedPointer<T> obj = qSharedPointerDynamicCast<T>(d->gObjects.value(id));
        if (obj.isNull())
        {
            throw VExceptionBadId(tr("Can't cast object"), id);
        }
        *obj = *point;
    }
    else
    {
        d->gObjects.insert(id, point);
    }
    UpdateId(id);
}
#endif // VCONTAINER_H
