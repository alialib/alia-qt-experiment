#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSettings>
#include <QTextEdit>
#include <QTimer>
#include <QWidget>

#include "adaptor.hpp"

using namespace alia;

using std::string;

template<class QtWidgetType>
struct widget_object : widget_layout_node, noncopyable
{
    typedef QtWidgetType qt_widget_type;

    widget_object(QtWidgetType* widget) : widget_(widget)
    {
        widget_ = widget;
        get_object(tree_node_).node = this;
    }
    widget_object() : widget_object(new QtWidgetType)
    {
    }

    ~widget_object()
    {
        if (widget_)
            widget_->deleteLater();
    }

    void
    refresh(dataless_qt_context ctx)
    {
        refresh_tree_node(alia::get<qt_traversal_tag>(ctx), tree_node_);
    }

    QtWidgetType*
    get() const
    {
        return widget_;
    }
    QtWidgetType&
    operator*()
    {
        return *widget_;
    }
    QtWidgetType*
    operator->()
    {
        return widget_;
    }

    // (required by widget_layout_node)
    QWidget*
    layout_widget() const override
    {
        return widget_;
    }

 private:
    QtWidgetType* widget_ = nullptr;
    tree_node<layout_object> tree_node_;
};

// TODO: Support signal parameters.
struct qt_signal_event : targeted_event
{
};

template<class Derived, class WidgetObject>
struct widget_handle_base
{
    widget_handle_base()
    {
    }

    widget_handle_base(qt_context ctx, WidgetObject& object, bool initializing)
        : object_(&object), initializing_(initializing)
    {
        ctx_.reset(ctx);
    }

    // // Specify a CONSTANT value for the 'class' attribute.
    // Derived&
    // classes(char const* value)
    // {
    //     return this->attr("class", value);
    // }
    // // Dynamically specify an individual token on the class attribute.
    // template<class Token>
    // Derived&
    // class_(Token token)
    // {
    //     detail::do_element_class_token(
    //         this->context(), this->node().object, this->initializing(),
    //         token);
    //     return static_cast<Derived&>(*this);
    // }
    // // Dynamically specify a conditional token on the class attribute.
    // template<class Token, class Condition>
    // Derived&
    // class_(Token token, Condition condition)
    // {
    //     detail::do_element_class_token(
    //         this->context(),
    //         this->node().object,
    //         this->initializing(),
    //         mask(std::move(token), std::move(condition)));
    //     return static_cast<Derived&>(*this);
    // }

    // // Specify the value of a property.
    // template<class Value>
    // Derived&
    // prop(char const* name, Value value)
    // {
    //     detail::do_element_property(
    //         this->context(), this->node().object, name, signalize(value));
    //     return static_cast<Derived&>(*this);
    // }

    // Specify a handler for a Qt signal.
    template<class QtSignal, class Function>
    Derived&
    handler(QtSignal signal, Function&& function)
    {
        // TODO: Support signal parameters.
        auto id = get_component_id(this->context());
        if (this->initializing())
        {
            auto& system = get<system_tag>(this->context());
            QObject::connect(&this->widget(), signal, [&system, id]() {
                qt_signal_event event;
                dispatch_targeted_event(system, event, externalize(id));
            });
        }
        targeted_event_handler<qt_signal_event>(
            this->context(), id, [&](auto ctx, auto& e) {
                std::forward<Function>(function)();
            });
        return static_cast<Derived&>(*this);
    }

    // Specify an action to perform in response to a Qt signal.
    template<class QtSignal>
    Derived&
    on(QtSignal signal, action<> const& action)
    {
        return handler(signal, [&]() { perform_action(action); });
    }

    // Specify a callback to call on element initialization.
    template<class Callback>
    Derived&
    init(Callback&& callback)
    {
        if (this->initializing())
            std::forward<Callback>(callback)(static_cast<Derived&>(*this));
        return static_cast<Derived&>(*this);
    }

    qt_context
    context()
    {
        return *ctx_;
    }
    WidgetObject&
    object()
    {
        return *object_;
    }
    typename WidgetObject::qt_widget_type&
    widget()
    {
        return *object_->get();
    }
    bool
    initializing()
    {
        return initializing_;
    }

 private:
    optional_context<qt_context> ctx_;
    WidgetObject* object_ = nullptr;
    bool initializing_;
};

template<class WidgetObject>
struct widget_handle
    : widget_handle_base<widget_handle<WidgetObject>, WidgetObject>
{
    using widget_handle_base::widget_handle_base;
};

struct qt_label : widget_object<QLabel>
{
    qt_label()
    {
        (*this)->setWordWrap(true);
    }

    captured_id text_id;
};

void
do_label(qt_context ctx, readable<string> text)
{
    auto& label = get_cached_data<qt_label>(ctx);

    refresh_handler(ctx, [&](auto ctx) {
        label.refresh(ctx);

        refresh_signal_view(
            label.text_id,
            text,
            [&](auto text) { label->setText(text.c_str()); },
            [&]() { label->setText(""); });
    });
}

struct click_event : targeted_event
{
};

struct qt_button : widget_object<QPushButton>
{
    captured_id text_id;
    component_identity identity;

    qt_button()
    {
    }
};

template<class Handle, class WidgetObject>
Handle
generic_widget(qt_context ctx)
{
    WidgetObject* object;
    bool initializing = get_cached_data(ctx, &object);
    refresh_handler(ctx, [&](auto ctx) { object->refresh(ctx); });
    return Handle(ctx, *object, initializing);
}

void
do_button(qt_context ctx, readable<string> text, action<> on_click)
{
    auto handle = generic_widget<widget_handle<qt_button>, qt_button>(ctx);
    auto& button = handle.object();

    refresh_handler(ctx, [&](auto ctx) {
        refresh_signal_view(
            button.text_id,
            text,
            [&](auto text) { button->setText(text.c_str()); },
            [&]() { button->setText(""); });
    });

    handle.on(&QPushButton::clicked, on_click);
}

struct value_update_event : targeted_event
{
    string value;
};

struct qt_text_control : widget_object<QTextEdit>
{
    captured_id text_id;
    component_identity identity;

    qt_text_control()
    {
    }
};

void
do_text_control(qt_context ctx, duplex<string> text)
{
    qt_text_control* widget_ptr;
    bool initializing = get_cached_data(ctx, &widget_ptr);
    auto& widget = *widget_ptr;

    if (initializing)
    {
        auto& system = get<system_tag>(ctx);
        QObject::connect(
            widget.get(),
            &QTextEdit::textChanged,
            // The Qt object is technically owned within both of these, so
            // I'm pretty sure it's safe to reference both.
            [&system, &widget]() {
                value_update_event event;
                event.value = widget->toPlainText().toUtf8().constData();
                dispatch_targeted_event(
                    system, event, externalize(&widget.identity));
            });
    }

    refresh_handler(ctx, [&](auto ctx) {
        widget.refresh(ctx);

        refresh_signal_view(
            widget.text_id,
            text,
            [&](auto text) {
                // Prevent update cycles.
                if (widget->toPlainText().toUtf8().constData() != text)
                {
                    widget->blockSignals(true);
                    widget->setText(text.c_str());
                    widget->blockSignals(false);
                }
            },
            [&]() {
                // Prevent update cycles.
                if (widget->toPlainText().toUtf8().constData() != "")
                {
                    widget->blockSignals(true);
                    widget->setText("");
                    widget->blockSignals(false);
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

struct qt_external_interface : default_external_interface
{
    qt_external_interface(alia::system& owner)
        : default_external_interface(owner)
    {
    }

    void
    schedule_animation_refresh()
    {
        // TODO: Deal with the possibility that the this object (or its owner)
        // is destroyed between now and when the timer fires.
        // TODO: Synchronize this with the monitor refresh, and/or adjust the
        // time so that it's relative to the start of the current refresh
        // event.
        QTimer::singleShot(10, [&] { refresh_system(this->owner); });
    }

    void
    schedule_timer_event(
        external_component_id component, millisecond_count time)
    {
        // TODO: Deal with the possibility that the this object (or its owner)
        // is destroyed between now and when the timer fires.
        QTimer::singleShot(
            time - this->get_tick_count(), [&, time, component] {
                timer_event event;
                event.trigger_time = time;
                dispatch_targeted_event(this->owner, event, component);
            });
    }
};

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
    // QObject::connect(qt_system.window, &QWidget::resizeEvent,
    // [&qt_system] {
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
    initialize_system(
        alia_system,
        std::ref(qt_system),
        new qt_external_interface(alia_system));

    // Do the initial refresh.
    refresh_system(alia_system);
}

qt_system::~qt_system()
{
    window->deleteLater();
    layout->deleteLater();
}

#include "moc_adaptor.cpp"
