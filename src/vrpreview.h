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

#ifndef VRPREVIEW_H
#define VRPREVIEW_H

#include <QObject>
#include <QString>
#include <QVector4D>

// Preview-only look-around view of a VR clip in the player. Changes what the player shows, never the
// project or the export. Conventions match the vr MLT module: +yaw = look right, +pitch = look up (degrees).
class VrPreview : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int eye READ eye WRITE setEye NOTIFY settingsChanged)
    Q_PROPERTY(int layout READ layout WRITE setLayout NOTIFY settingsChanged)
    Q_PROPERTY(int effectiveLayout READ effectiveLayout NOTIFY settingsChanged)
    Q_PROPERTY(QString layoutName READ layoutName NOTIFY settingsChanged)
    Q_PROPERTY(bool full360 READ full360 NOTIFY settingsChanged)
    Q_PROPERTY(QVector4D leftRect READ leftRect NOTIFY settingsChanged)
    Q_PROPERTY(QVector4D rightRect READ rightRect NOTIFY settingsChanged)
    Q_PROPERTY(QVector4D lonLatRange READ lonLatRange NOTIFY settingsChanged)
    Q_PROPERTY(double yaw READ yaw NOTIFY viewChanged)
    Q_PROPERTY(double pitch READ pitch NOTIFY viewChanged)
    Q_PROPERTY(double fov READ fov NOTIFY viewChanged)

public:
    enum Eye { LeftEye = 0, RightEye = 1, Anaglyph = 2 };
    enum Layout {
        Auto = -1,
        Vr180SideBySide = 0,
        Vr180TopBottom,
        Vr180Mono,
        Vr360Mono,
        Vr360TopBottom,
        Vr360SideBySide,
        LayoutCount
    };
    Q_ENUM(Eye)
    Q_ENUM(Layout)

    static VrPreview &singleton();
    static QString layoutLabel(int layout);

    bool enabled() const { return m_enabled; }
    int eye() const { return m_eye; }
    int layout() const { return m_layout; }
    int effectiveLayout() const;
    QString layoutName() const { return layoutLabel(effectiveLayout()); }
    bool full360() const;
    QVector4D leftRect() const;  // normalized x, y, w, h of the left eye inside the frame
    QVector4D rightRect() const; // same for the right eye (equals left for mono)
    QVector4D lonLatRange() const; // lon0, lon1, lat0, lat1 in radians
    double yaw() const { return m_yaw; }
    double pitch() const { return m_pitch; }
    double fov() const { return m_fov; }

public slots:
    void setEnabled(bool enabled);
    void setEye(int eye);
    void setLayout(int layout);
    void look(double yaw, double pitch, double fov);
    void faceFront();

signals:
    void enabledChanged(bool enabled);
    void settingsChanged();
    void viewChanged();

private:
    explicit VrPreview(QObject *parent = nullptr);
    QVector4D eyeRect(bool right) const;

    bool m_enabled;
    int m_eye;
    int m_layout;
    double m_yaw;
    double m_pitch;
    double m_fov;
};

#endif // VRPREVIEW_H
