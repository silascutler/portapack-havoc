/*
 * Copyright (C) 2014 Jared Boone, ShareBrained Technology, Inc.
 * Copyright (C) 2016 Furrtek
 *
 * This file is part of PortaPack.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __UI_WIDGET_H__
#define __UI_WIDGET_H__

#include "ui.hpp"
#include "ui_text.hpp"
#include "ui_painter.hpp"
#include "ui_focus.hpp"
#include "ui_font_fixed_8x16.hpp"
#include "radio.hpp"

#include "portapack.hpp"
#include "utility.hpp"

#include <memory>
#include <vector>
#include <string>

namespace ui {

void dirty_set();
void dirty_clear();
bool is_dirty();

class Context {
public:
	FocusManager& focus_manager() {
		return focus_manager_;
	}

private:
	FocusManager focus_manager_ { };
};

class Widget {
public:
	Widget(
	) : _parent_rect { }
	{
	}

	Widget(
		Rect parent_rect
	) : _parent_rect { parent_rect }
	{
	}

	Widget(const Widget&) = delete;
	Widget(Widget&&) = delete;
	Widget& operator=(const Widget&) = delete;
	Widget& operator=(Widget&&) = delete;

	virtual ~Widget() = default;

	Point screen_pos();
	Size size() const;
	Rect screen_rect() const;
	Rect parent_rect() const;
	virtual void set_parent_rect(const Rect new_parent_rect);

	Widget* parent() const;
	void set_parent(Widget* const widget);

	bool hidden() const { return flags.hidden; }
	void hidden(bool hide);

	virtual void focus();
	virtual void on_focus();
	virtual void blur();
	virtual void on_blur();
	bool focusable() const;
	void set_focusable(const bool value);
	bool has_focus();

	virtual void paint(Painter& painter) = 0;

	virtual void on_show() { };
	virtual void on_hide() { };

	virtual bool on_key(const KeyEvent event);
	virtual bool on_encoder(const EncoderEvent event);
	virtual bool on_touch(const TouchEvent event);
	virtual const std::vector<Widget*>& children() const;

	virtual Context& context() const;

	void set_style(const Style* new_style);

	const Style& style() const;
	uint16_t id = 0;

	// State management methods.
	void set_dirty();
	bool dirty() const;
	void set_clean();

	void visible(bool v);
	bool visible() { return flags.visible; };

	bool highlighted() const;
	void set_highlighted(const bool value);

protected:
	void dirty_overlapping_children_in_rect(const Rect& child_rect);

private:
	/* Widget rectangle relative to parent pos(). */
	Rect _parent_rect;
	const Style* style_ { nullptr };
	Widget* parent_ { nullptr };

	struct flags_t {
		bool dirty : 1;			// Widget content has changed.
		bool hidden : 1;		// Hide widget and children.
		bool focusable : 1;		// Widget can receive focus.
		bool highlighted : 1;	// Show in a highlighted style.
		bool visible : 1;		// Object was visible during last paint.
	};

	flags_t flags {
		.dirty = true,
		.hidden = false,
		.focusable = false,
		.highlighted = false,
		.visible = false,
	};

	static const std::vector<Widget*> no_children;
};

class View : public Widget {
public:
	View() {
	}

	View(Rect parent_rect) {
		set_parent_rect(parent_rect);
	}

	// TODO: ~View() should on_hide() all children?

	void paint(Painter& painter) override;

	void add_child(Widget* const widget);
	void add_children(const std::initializer_list<Widget*> children);
	void remove_child(Widget* const widget);
	void remove_children(const std::vector<Widget*>& children);
	const std::vector<Widget*>& children() const override;

	virtual std::string title() const;

protected:
	std::vector<Widget*> children_ { };

	void invalidate_child(Widget* const widget);
};

class Rectangle : public Widget {
public:
	Rectangle(Color c);
	Rectangle(Rect parent_rect, Color c);
	Rectangle(
	) : Rectangle { { }, { } }
	{
	}

	void paint(Painter& painter) override;

	void set_color(const Color c);
	void set_outline(const bool outline);

private:
	Color color;
	bool _outline = false;
};

class Text : public Widget {
public:
	Text(
	) : text { "" } {
	}

	Text(Rect parent_rect, std::string text);
	Text(Rect parent_rect);

	void set(const std::string value);

	void paint(Painter& painter) override;

private:
	std::string text;
};

class Labels : public Widget {
public:
	struct Label {
		Point pos;
		std::string text;
		ui::Color color;
	};
	
	Labels(const Labels&) = delete;
	Labels(Labels&&) = delete;
	Labels& operator=(const Labels&) = delete;
	Labels& operator=(Labels&&) = delete;

	Labels(std::initializer_list<Label> labels);

	void set_labels(std::initializer_list<Label> labels);

	void paint(Painter& painter) override;

private:
	std::vector<Label> labels_;
};

class BigFrequency : public Widget {
public:
	BigFrequency(Rect parent_rect, rf::Frequency frequency);

	void set(const rf::Frequency frequency);

	void paint(Painter& painter) override;

private:
	rf::Frequency _frequency;
};

class ProgressBar : public Widget {
public:
	ProgressBar(Rect parent_rect);

	void set_max(const uint32_t max);
	void set_value(const uint32_t value);

	void paint(Painter& painter) override;

private:
	uint32_t _value = 0;
	uint32_t _max = 100;
};

class Console : public Widget {
public:
	Console(Rect parent_rect);
	
	void clear();
	void write(std::string message);
	void writeln(std::string message);

	void paint(Painter&) override;
	
	void on_show() override;
	void on_hide() override;

private:
	bool visible = false;
	Point pos { 0, 0 };
	std::string buffer { };

	void crlf();
};

class Checkbox : public Widget {
public:
	std::function<void(Checkbox&, bool)> on_select { };

	Checkbox(Point parent_pos, size_t length, std::string text, bool small);
	Checkbox(
		Point parent_pos,
		size_t length,
		std::string text
	) : Checkbox { parent_pos, length, text, false }
	{
	}
	
	Checkbox(
	) : Checkbox { { }, { }, { } }
	{
	}
	
	Checkbox(const Checkbox&) = delete;
	Checkbox(Checkbox&&) = delete;
	Checkbox& operator=(const Checkbox&) = delete;
	Checkbox& operator=(Checkbox&&) = delete;
	
	void set_text(const std::string value);
	bool set_value(const bool value);
	bool value() const;

	void paint(Painter& painter) override;
	
	bool on_key(const KeyEvent key) override;
	bool on_touch(const TouchEvent event) override;

private:
	std::string text_;
	bool small_ { false };
	bool value_ { false };
	const Style * style_ { nullptr };
};

class Button : public Widget {
public:
	std::function<void(Button&)> on_select { };
	std::function<bool(Button&, KeyEvent)> on_dir { };
	std::function<void(Button&)> on_highlight { };

	Button(Rect parent_rect, std::string text);

	Button(
	) : Button { { }, { } }
	{
	}

	void set_text(const std::string value);
	std::string text() const;

	void paint(Painter& painter) override;

	void on_focus() override;
	bool on_key(const KeyEvent key) override;
	bool on_touch(const TouchEvent event) override;

private:
	std::string text_;
};

class Image : public Widget {
public:
	Image();
	Image(
		const Rect parent_rect,
		const Bitmap* bitmap,
		const Color foreground,
		const Color background
	);

	Image(const Image&) = delete;
	Image(Image&&) = delete;
	Image& operator=(const Image&) = delete;
	Image& operator=(Image&&) = delete;

	void set_bitmap(const Bitmap* bitmap);
	void set_foreground(const Color color);
	void set_background(const Color color);
	void invert_colors();

	void paint(Painter& painter) override;

private:
	const Bitmap* bitmap_;
	Color foreground_;
	Color background_;
};

class ImageButton : public Image {
public:
	std::function<void(ImageButton&)> on_select { };

	ImageButton(
		const Rect parent_rect,
		const Bitmap* bitmap,
		const Color foreground,
		const Color background
	);

	bool on_key(const KeyEvent key) override;
	bool on_touch(const TouchEvent event) override;
};

class ImageOptionsField : public Widget {
public:
	using image_t = const unsigned char *;
	using value_t = int32_t;
	using option_t = std::pair<image_t, value_t>;
	using options_t = std::vector<option_t>;

	std::function<void(size_t, value_t)> on_change { };
	std::function<void(void)> on_show_options { };

	ImageOptionsField(Rect parent_rect, options_t options);
	
	ImageOptionsField(
	) : ImageOptionsField { { }, { } }
	{
	}
	
	void set_options(options_t new_options);

	size_t selected_index() const;
	size_t selected_index_value() const;
	void set_selected_index(const size_t new_index);

	void set_by_value(value_t v);

	void paint(Painter& painter) override;

	void on_focus() override;
	bool on_encoder(const EncoderEvent delta) override;
	bool on_touch(const TouchEvent event) override;

private:
	options_t options;
	size_t selected_index_ { 0 };
};

class OptionsField : public Widget {
public:
	using name_t = std::string;
	using value_t = int32_t;
	using option_t = std::pair<name_t, value_t>;
	using options_t = std::vector<option_t>;

	std::function<void(size_t, value_t)> on_change { };
	std::function<void(void)> on_show_options { };

	OptionsField(Point parent_pos, int length, options_t options);
	
	void set_options(options_t new_options);

	size_t selected_index() const;
	size_t selected_index_value() const;
	void set_selected_index(const size_t new_index);

	void set_by_value(value_t v);

	void paint(Painter& painter) override;

	void on_focus() override;
	bool on_encoder(const EncoderEvent delta) override;
	bool on_touch(const TouchEvent event) override;

private:
	const int length_;
	options_t options;
	size_t selected_index_ { 0 };
};

class NumberField : public Widget {
public:
	std::function<void(NumberField&)> on_select { };
	std::function<void(int32_t)> on_change { };

	using range_t = std::pair<int32_t, int32_t>;

	NumberField(Point parent_pos, int length, range_t range, int32_t step, char fill_char);
	NumberField(
	) : NumberField { { 0, 0 }, 1, { 0, 1 }, 1, ' ' }
	{
	}

	NumberField(const NumberField&) = delete;
	NumberField(NumberField&&) = delete;

	int32_t value() const;
	void set_value(int32_t new_value, bool trigger_change = true);
	void set_range(const int32_t min, const int32_t max);

	void paint(Painter& painter) override;

	bool on_key(const KeyEvent key) override;
	bool on_encoder(const EncoderEvent delta) override;
	bool on_touch(const TouchEvent event) override;

private:
	range_t range;
	const int32_t step;
	const int length_;
	const char fill_char;
	int32_t value_ { 0 };

	int32_t clip_value(int32_t value);
};

class SymField : public Widget {
public:
	std::function<void(SymField&)> on_select { };
	std::function<void()> on_change { };

	enum symfield_type {
		SYMFIELD_OCT,
		SYMFIELD_DEC,
		SYMFIELD_HEX,
		SYMFIELD_DEF		// User DEFined
	};
	
	SymField(Point parent_pos, size_t length, symfield_type type);

	SymField(const SymField&) = delete;
	SymField(SymField&&) = delete;

	uint32_t value(const uint32_t index);
	void set_value(const uint32_t index, const uint32_t new_value);
	void set_length(const uint32_t new_length);
	void set_symbol_list(const uint32_t index, const std::string symbol_list);
	uint32_t value_dec_u32();
	uint64_t value_hex_u64();

	void paint(Painter& painter) override;

	bool on_key(const KeyEvent key) override;
	bool on_encoder(const EncoderEvent delta) override;
	bool on_touch(const TouchEvent event) override;

private:
	std::string symbol_list_[32] = { "01" };		// Failsafe init
	uint32_t values_[32] = { 0 };
	uint32_t selected_ = 0;
	size_t length_, prev_length_ = 0;
	bool erase_prev_ = false;
	symfield_type type_ { };

	int32_t clip_value(const uint32_t index, const uint32_t value);
};

class Waveform : public Widget {
public:

	Waveform(Rect parent_rect, int8_t * data, uint32_t length, uint32_t offset, bool digital, Color color);

	Waveform(const Waveform&) = delete;
	Waveform(Waveform&&) = delete;
	Waveform& operator=(const Waveform&) = delete;
	Waveform& operator=(Waveform&&) = delete;

	void set_offset(const uint32_t new_offset);
	void set_length(const uint32_t new_length);

	void paint(Painter& painter) override;

private:
	int8_t * data_;
	uint32_t length_;
	uint32_t offset_;
	bool digital_ { false };
	Color color_;
};

} /* namespace ui */

#endif/*__UI_WIDGET_H__*/
