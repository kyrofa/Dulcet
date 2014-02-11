#ifndef DULCET_PLUGIN_INTERFACE_H
#define DULCET_PLUGIN_INTERFACE_H

#include <QSettings>

class Q_DECL_EXPORT DulcetPluginInterface : public QObject
{
	Q_OBJECT

	public:
		Q_DECL_EXPORT explicit DulcetPluginInterface(QObject *parent = 0) : QObject(parent) {}
		Q_DECL_EXPORT virtual ~DulcetPluginInterface() {}

		Q_DECL_EXPORT virtual void saveSettings(QSettings &settings) {Q_UNUSED(settings)}
		Q_DECL_EXPORT virtual void loadSettings(const QSettings &settings) {Q_UNUSED(settings)}
};

Q_DECLARE_INTERFACE(DulcetPluginInterface, "dulcet.plugins.DulcetPluginInterface")

#endif // DULCET_PLUGIN_INTERFACE_H
