// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SEAMLY3D_DRESSFORM_H
#define SEAMLY3D_DRESSFORM_H
#include "model.h"
#include <QVector2D>
namespace Seamly3D
{
bool validateDressForm(const DressForm &form, QString &error);
double dressFormBottom(const DressForm &form);
double dressFormTop(const DressForm &form);
double dressFormFloor(const DressForm &form);
QVector2D dressFormRadii(const DressForm &form, double y);
// Uniform-height rings (y, x radius, z radius), shared by rendering and contact.
QVector<QVector3D> dressFormProfile(const DressForm &form);
Mesh dressFormMesh(const DressForm &form);
QVector3D outsideDressForm(const QVector<QVector3D> &profile, const QVector3D &point, float clearance);
void placeAroundDressForm(Piece &piece, const DressForm &form, double angleDegrees);
}
#endif
