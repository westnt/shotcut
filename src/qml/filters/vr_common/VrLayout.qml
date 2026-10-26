// SPDX-License-Identifier: GPL-2.0-or-later
// Shared "VR clip layout" controls for every vr_* filter panel: packing, coverage, eye order.
// Writes the MLT properties read by port/mlt/vr/vr_props.hpp (packing, lon_min..lat_max, swap_eyes).
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Shotcut.Controls as Shotcut

RowLayout {
    id: root

    property bool blockUpdate: false

    function load() {
        blockUpdate = true;
        var p = filter.get('packing');
        packingCombo.currentIndex = p === 'mono' ? 0 : (p === 'tb' ? 2 : 1);
        coverageCombo.currentIndex = filter.getDouble('lon_max') >= 179 ? 1 : 0;
        swapCheck.checked = filter.get('swap_eyes') === '1';
        blockUpdate = false;
    }

    function save() {
        if (blockUpdate)
            return;
        filter.set('packing', ['mono', 'sbs', 'tb'][packingCombo.currentIndex]);
        var full = coverageCombo.currentIndex === 1;
        filter.set('lon_min', full ? -180 : -90);
        filter.set('lon_max', full ? 180 : 90);
        filter.set('lat_min', -90);
        filter.set('lat_max', 90);
        filter.set('swap_eyes', swapCheck.checked ? 1 : 0);
    }

    Component.onCompleted: {
        if (filter.isNew) {
            filter.set('packing', 'sbs');
            filter.set('lon_min', -90);
            filter.set('lon_max', 90);
        }
        load();
    }

    Shotcut.ComboBox {
        id: packingCombo

        model: [qsTr('Mono'), qsTr('Side by side'), qsTr('Top / bottom')]
        onActivated: root.save()
    }

    Shotcut.ComboBox {
        id: coverageCombo

        model: [qsTr('180°'), qsTr('360°')]
        onActivated: root.save()
    }

    CheckBox {
        id: swapCheck

        text: qsTr('Right eye first')
        onClicked: root.save()
    }
}
