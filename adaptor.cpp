#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSettings>
#include <QTextEdit>
#include <QWidget>

#include "adaptor.hpp"

using namespace alia;

using std::string;

struct qt_label
{
    QLabel* object = nullptr;
    captured_id text_id;
    alia::tree_node<layout_object> tree_node;
    widget_layout_node layout_node;

    ~qt_label()
    {
        if (object)
            object->deleteLater();
    }
};

void
do_label(qt_context ctx, readable<string> text)
{
    auto& label = get_cached_data<qt_label>(ctx);

    refresh_handler(ctx, [&](auto ctx) {
        auto& system = get<system_tag>(ctx);

        if (!label.object)
        {
            auto& traversal = get<qt_traversal_tag>(ctx);
            label.object = new QLabel;
            label.object->setWordWrap(true);
            label.layout_node.initialize(label.object);
            label.tree_node.object.node = &label.layout_node;
        }

        refresh_tree_node(get<qt_traversal_tag>(ctx), label.tree_node);

        refresh_signal_view(
            label.text_id,
            text,
            [&](auto text) { label.object->setText(text.c_str()); },
            [&]() { label.object->setText(""); });
    });
}

struct click_event : targeted_event
{
};

struct qt_button
{
    QPushButton* object = nullptr;
    captured_id text_id;
    component_identity identity;
    alia::tree_node<layout_object> tree_node;
    widget_layout_node layout_node;

    ~qt_button()
    {
        if (object)
            object->deleteLater();
    }
};

void
do_button(qt_context ctx, readable<string> text, action<> on_click)
{
    auto& button = get_cached_data<qt_button>(ctx);

    refresh_handler(ctx, [&](auto ctx) {
        auto& system = get<system_tag>(ctx);

        refresh_component_identity(ctx, button.identity);

        if (!button.object)
        {
            auto& traversal = get<qt_traversal_tag>(ctx);
            button.object = new QPushButton;
            button.layout_node.initialize(button.object);
            button.tree_node.object.node = &button.layout_node;
            QObject::connect(
                button.object,
                &QPushButton::clicked,
                // The Qt object is technically owned within both of these, so
                // I'm pretty sure it's safe to reference both.
                [&system, &button]() {
                    click_event event;
                    dispatch_targeted_event(
                        system, event, externalize(&button.identity));
                });
        }

        refresh_tree_node(get<qt_traversal_tag>(ctx), button.tree_node);

        refresh_signal_view(
            button.text_id,
            text,
            [&](auto text) { button.object->setText(text.c_str()); },
            [&]() { button.object->setText(""); });
    });

    targeted_event_handler<click_event>(
        ctx, &button.identity, [&](auto ctx, auto& e) {
            if (action_is_ready(on_click))
            {
                perform_action(on_click);
            }
        });
}

struct value_update_event : targeted_event
{
    string value;
};

struct qt_text_control
{
    QTextEdit* object = nullptr;
    captured_id text_id;
    component_identity identity;
    alia::tree_node<layout_object> tree_node;
    widget_layout_node layout_node;

    ~qt_text_control()
    {
        if (object)
            object->deleteLater();
    }
};

void
do_text_control(qt_context ctx, duplex<string> text)
{
    auto& widget = get_cached_data<qt_text_control>(ctx);

    refresh_handler(ctx, [&](auto ctx) {
        auto& system = get<system_tag>(ctx);

        refresh_component_identity(ctx, widget.identity);

        if (!widget.object)
        {
            auto& traversal = get<qt_traversal_tag>(ctx);
            widget.object = new QTextEdit;
            widget.layout_node.initialize(widget.object);
            widget.tree_node.object.node = &widget.layout_node;
            QObject::connect(
                widget.object,
                &QTextEdit::textChanged,
                // The Qt object is technically owned within both of these, so
                // I'm pretty sure it's safe to reference both.
                [&system, &widget]() {
                    value_update_event event;
                    event.value
                        = widget.object->toPlainText().toUtf8().constData();
                    dispatch_targeted_event(
                        system, event, externalize(&widget.identity));
                });
        }

        refresh_tree_node(get<qt_traversal_tag>(ctx), widget.tree_node);

        refresh_signal_view(
            widget.text_id,
            text,
            [&](auto text) {
                // Prevent update cycles.
                if (widget.object->toPlainText().toUtf8().constData() != text)
                {
                    widget.object->blockSignals(true);
                    widget.object->setText(text.c_str());
                    widget.object->blockSignals(false);
                }
            },
            [&]() {
                // Prevent update cycles.
                if (widget.object->toPlainText().toUtf8().constData() != "")
                {
                    widget.object->blockSignals(true);
                    widget.object->setText("");
                    widget.object->blockSignals(false);
                }
            });
    });

    targeted_event_handler<value_update_event>(
        ctx, &widget.identity, [&](auto ctx, auto& e) {
            write_signal(text, e.value);
        });
}

struct qt_column
{
    QVBoxLayout* qt_object = nullptr;
    alia::tree_node<layout_object> tree_node;
    box_layout tree_object;

    qt_column()
    {
        qt_object = new QVBoxLayout;
        tree_object.initialize(qt_object);
        tree_node.object.node = &tree_object;
    }

    ~qt_column()
    {
        if (qt_object)
            qt_object->deleteLater();
    }
};

void
scoped_column::begin(qt_context ctx)
{
    qt_column* column;
    get_cached_data(ctx, &column);
    if (is_refresh_event(ctx))
        tree_scoping_.begin(get<qt_traversal_tag>(ctx), column->tree_node);
}
void
scoped_column::end()
{
    tree_scoping_.end();
}

void
qt_system::operator()(alia::context vanilla_ctx)
{
    tree_traversal<layout_object> traversal;
    qt_context ctx = extend_context<qt_traversal_tag>(vanilla_ctx, traversal);

    scoped_tree_root<layout_object> scoped_root;
    if (is_refresh_event(ctx))
        scoped_root.begin(traversal, this->tree_root);

    this->controller(ctx);
}

void
initialize(
    qt_system& qt_system,
    alia::system& alia_system,
    std::function<void(qt_context)> controller)
{
    // Initialize the Qt system.
    qt_system.system = &alia_system;
    MainWindow* window = new MainWindow;
    window->readSettings();
    qt_system.window = window;

    // {
    //     QSettings settings("alia", "Qt");
    //     qt_system.window->restoreGeometry(
    //         settings.value("geometry").toByteArray());
    // }
    // QObject::connect(qt_system.window, &QWidget::resizeEvent, [&qt_system] {
    //     QSettings settings("alia", "Qt");
    //     settings.setValue("geometry", qt_system.window->saveGeometry());
    // });

    window->setProperty("bgType", "toplevel");

    qt_system.layout = new QVBoxLayout(qt_system.window);
    qt_system.window->setLayout(qt_system.layout);
    qt_system.controller = std::move(controller);
    qt_system.layout_root.initialize(qt_system.layout);
    qt_system.tree_root.object.node = &qt_system.layout_root;

    // Hook up the Qt system to the alia system.
    initialize_system(alia_system, std::ref(qt_system));

    // Do the initial refresh.
    refresh_system(alia_system);
}

qt_system::~qt_system()
{
    window->deleteLater();
    layout->deleteLater();
}

#include "moc_adaptor.cpp"
