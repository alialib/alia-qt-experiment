#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include <QWidget>

#include <alia.hpp>

#include "adaptor.hpp"

using namespace alia;

void
do_sandbox_ui(qt_context ctx)
{
    scoped_column row(ctx);

    do_label(
        ctx,
        value(
            "<p style=\"line-height:120%;\">Lorem Ipsum is simply dummy text "
            "of the printing and "
            "typesetting industry. Lorem Ipsum has been the industry's "
            "standard dummy text ever since <b>the 1500s</b>, when an unknown "
            "printer took a galley of type and scrambled it to make a type "
            "specimen book. It has survived not only five centuries, but "
            "also the leap into electronic typesetting, remaining "
            "essentially unchanged. It was popularised in the 1960s with "
            "the release of Letraset sheets containing Lorem Ipsum "
            "passages, and more recently with desktop publishing software "
            "like Aldus PageMaker including versions of Lorem Ipsum.</p>"));

    // auto x = get_state(ctx, empty<std::string>());
    // do_text_control(ctx, x);
    // do_text_control(ctx, x);

    // do_label(ctx, x);

    // auto state = get_state(ctx, true);
    // ALIA_IF(state)
    // {
    //     do_label(ctx, value("Secret message!"));
    // }
    // ALIA_END

    // do_button(ctx, value("Reset"), x <<= "abacadaba!");
    // do_button(ctx, x, actions::toggle(state));
    // do_button(ctx, value("Toggle!"), actions::toggle(state));
}

alia::system the_system;
qt_system the_qt;

int
main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QFontDatabase::addApplicationFont("Roboto/Roboto-Regular.ttf");

    QFile file("style.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    app.setStyleSheet(styleSheet);

    initialize(the_qt, the_system, do_sandbox_ui);

    refresh_system(the_system);

    the_qt.window->setWindowTitle("alia Qt");
    the_qt.window->show();

    return app.exec();
}
