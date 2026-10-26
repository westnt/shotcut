/*
 * Copyright (c) 2026
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "vrpreview.h"

#include "mltcontroller.h"
#include "qmltypes/qmlprofile.h"

#include <QSettings>
#include <QtMath>

static const double kMinFov = 20.0;
static const double kMaxFov = 140.0;
static const double kDefaultFov = 90.0;

VrPreview &VrPreview::singleton()
{
    static VrPreview instance;
    return instance;
}

VrPreview::VrPreview(QObject *parent)
    : QObject(parent)
    , m_enabled(false)
    , m_yaw(0.0)
    , m_pitch(0.0)
{
    QSettings settings;
    m_eye = qBound(0, settings.value("player/vrEye", LeftEye).toInt(), 2);
    m_layout = qBound(int(Auto), settings.value("player/vrLayout", Auto).toInt(), LayoutCount - 1);
    m_fov = qBound(kMinFov, settings.value("player/vrFov", kDefaultFov).toDouble(), kMaxFov);
    // Auto layout depends on the project profile.
    connect(&QmlProfile::singleton(), &QmlProfile::profileChanged, this, &VrPreview::settingsChanged);
}

QString VrPreview::layoutLabel(int layout)
{
    switch (layout) {
    case Vr180SideBySide:
        return tr("VR180 side by side");
    case Vr180TopBottom:
        return tr("VR180 top / bottom");
    case Vr180Mono:
        return tr("VR180 mono");
    case Vr360Mono:
        return tr("360° mono");
    case Vr360TopBottom:
        return tr("360° top / bottom");
    case Vr360SideBySide:
        return tr("360° side by side");
    default:
        return tr("Auto-detect");
    }
}

int VrPreview::effectiveLayout() const
{
    if (m_layout != Auto)
        return m_layout;
    // From the profile: frame aspect plus hints in its name ("8K VR 180 LR 59.94", "360 TB", ...).
    const QString name = QString::fromUtf8(MLT.profile().description()).toLower();
    const double w = MLT.profile().width();
    const double h = qMax(1, MLT.profile().height());
    const double aspect = w / h;
    const bool says360 = name.contains("360");
    const bool says180 = name.contains("180");
    const bool saysTB = name.contains("tb") || name.contains("top");
    if (says360 && !says180) {
        if (aspect > 3.0)
            return Vr360SideBySide;
        if (aspect < 1.5 || saysTB)
            return Vr360TopBottom;
        return Vr360Mono;
    }
    if (aspect < 0.75)
        return Vr180TopBottom;
    if (aspect < 1.5)
        return saysTB ? Vr180TopBottom : Vr180Mono;
    return Vr180SideBySide;
}

bool VrPreview::full360() const
{
    const int l = effectiveLayout();
    return l == Vr360Mono || l == Vr360TopBottom || l == Vr360SideBySide;
}

QVector4D VrPreview::eyeRect(bool right) const
{
    switch (effectiveLayout()) {
    case Vr180SideBySide:
    case Vr360SideBySide:
        return right ? QVector4D(0.5f, 0.0f, 0.5f, 1.0f) : QVector4D(0.0f, 0.0f, 0.5f, 1.0f);
    case Vr180TopBottom:
    case Vr360TopBottom:
        return right ? QVector4D(0.0f, 0.5f, 1.0f, 0.5f) : QVector4D(0.0f, 0.0f, 1.0f, 0.5f);
    default:
        return QVector4D(0.0f, 0.0f, 1.0f, 1.0f);
    }
}

QVector4D VrPreview::leftRect() const
{
    return eyeRect(false);
}

QVector4D VrPreview::rightRect() const
{
    return eyeRect(true);
}

QVector4D VrPreview::lonLatRange() const
{
    const float half = full360() ? float(M_PI) : float(M_PI_2);
    return QVector4D(-half, half, -float(M_PI_2), float(M_PI_2));
}

void VrPreview::setEnabled(bool enabled)
{
    if (enabled == m_enabled)
        return;
    m_enabled = enabled;
    emit enabledChanged(enabled);
}

void VrPreview::setEye(int eye)
{
    eye = qBound(0, eye, 2);
    if (eye == m_eye)
        return;
    m_eye = eye;
    QSettings().setValue("player/vrEye", eye);
    emit settingsChanged();
}

void VrPreview::setLayout(int layout)
{
    layout = qBound(int(Auto), layout, LayoutCount - 1);
    if (layout == m_layout)
        return;
    m_layout = layout;
    QSettings().setValue("player/vrLayout", layout);
    emit settingsChanged();
}

void VrPreview::look(double yaw, double pitch, double fov)
{
    // Keep yaw in (-180, 180] so the readout stays readable after many turns.
    yaw = std::remainder(yaw, 360.0);
    pitch = qBound(-90.0, pitch, 90.0);
    fov = qBound(kMinFov, fov, kMaxFov);
    if (yaw == m_yaw && pitch == m_pitch && fov == m_fov)
        return;
    const bool fovChanged = fov != m_fov;
    m_yaw = yaw;
    m_pitch = pitch;
    m_fov = fov;
    if (fovChanged)
        QSettings().setValue("player/vrFov", fov);
    emit viewChanged();
}

void VrPreview::faceFront()
{
    look(0.0, 0.0, kDefaultFov);
}
