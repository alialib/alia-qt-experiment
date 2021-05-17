#ifndef QT_ADAPTOR_HPP
#define QT_ADAPTOR_HPP

#include <alia.hpp>

#include <QSettings>
#include <QWidget>

#include "layout.hpp"

ALIA_DEFINE_TAGGED_TYPE(qt_traversal_tag, alia::tree_traversal<layout_object>&)

typedef alia::extend_context_type_t<alia::context, qt_traversal_tag>
    qt_context;

typedef alia::remove_context_tag_t<qt_context, alia::data_traversal_tag>
    dataless_qt_context;

void
label(qt_context ctx, alia::readable<std::string> text);

void
line_edit(qt_context ctx, alia::duplex<std::string> text);

void
text_edit(qt_context ctx, alia::duplex<std::string> text);

void
button(
    qt_context ctx, alia::readable<std::string> text, alia::action<> on_click);

void
checkbox(
    qt_context ctx,
    alia::duplex<bool> checked,
    alia::readable<std::string> label);

struct layout_container;

struct scoped_scroll_area
{
    scoped_scroll_area()
    {
    }
    scoped_scroll_area(qt_context ctx)
    {
        begin(ctx);
    }
    ~scoped_scroll_area()
    {
        end();
    }

    void
    begin(qt_context ctx);

    void
    end();

 private:
    alia::scoped_tree_node<layout_object> tree_scoping_;
};

struct scoped_row
{
    scoped_row()
    {
    }
    scoped_row(qt_context ctx)
    {
        begin(ctx);
    }
    ~scoped_row()
    {
        end();
    }

    void
    begin(qt_context ctx);

    void
    end();

 private:
    alia::scoped_tree_node<layout_object> tree_scoping_;
};

template<class Content>
void
row(qt_context ctx, Content&& content)
{
    scoped_row scoped(ctx);
    std::forward<Content>(content)();
}

struct scoped_column
{
    scoped_column()
    {
    }
    scoped_column(qt_context ctx)
    {
        begin(ctx);
    }
    ~scoped_column()
    {
        end();
    }

    void
    begin(qt_context ctx);

    void
    end();

 private:
    alia::scoped_tree_node<layout_object> tree_scoping_;
};

template<class Content>
void
column(qt_context ctx, Content&& content)
{
    scoped_column scoped(ctx);
    std::forward<Content>(content)();
}

class QWidget;
class QVBoxLayout;

struct qt_system
{
    alia::system* system = nullptr;

    std::function<void(qt_context)> controller;

    // the top-level window and layout for the UI - The entire application's UI
    // tree lives inside this.
    QWidget* window = nullptr;
    QVBoxLayout* layout = nullptr;

    // the root of the layout tree
    alia::tree_node<layout_object> tree_root;
    box_layout_node layout_root;

    void
    operator()(alia::context ctx);

    ~qt_system();
};

void
initialize(
    qt_system& qt_system,
    alia::system& alia_system,
    std::function<void(qt_context)> controller);

class MainWindow : public QWidget
{
    Q_OBJECT

 public:
    void
    MainWindow::readSettings()
    {
        QSettings settings("alia", "qt-experiment");
        restoreGeometry(settings.value("geometry").toByteArray());
    }

 private:
    void
    closeEvent(QCloseEvent* event)
    {
        QSettings settings("alia", "qt-experiment");
        settings.setValue("geometry", saveGeometry());
        QWidget::closeEvent(event);
    }
};

#endif
