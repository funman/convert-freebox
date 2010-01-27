/*
 * Convert.cpp : GUI to libvlc-based video converter
 *
 * Copyright © 2009 Rafaël Carré
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <QFileDialog>

#include "gui/Convert.hpp"
#include "gui/ConvertThread.hpp"
#include "src/convert.h"

/* Convert : main GUI / control class */

Convert::Convert(void)
{
    libvlc_loaded = false;

    ui.setupUi(this);

    /* choose file buttons */
    connect( ui.input_choose, SIGNAL(clicked()), this, SLOT( input_choose()));
    connect(ui.output_choose, SIGNAL(clicked()), this, SLOT(output_choose()));

    /* file text box */
    connect( ui.input_file, SIGNAL(textEdited(const QString &)),
            this, SLOT( input_check(const QString &)));
    connect(ui.output_file, SIGNAL(textEdited(const QString &)),
            this, SLOT(output_check(const QString &)));

    /* convert button */
    connect(ui.convert_button, SIGNAL(clicked()), this, SLOT(convert_start()));

    show();
}

Convert::~Convert(void)
{
    if(libvlc_loaded)
        convert_exit();
}

void Convert::update_progress(float progress)
{
    ui.progressBar->setValue(progress * 100);
}

void Convert::status(QString msg)
{
    statusBar()->showMessage(msg);
}

void Convert::finished(void)
{
    ui.input_file->setText(NULL);
    ui.output_file->setText(NULL);
    ui.input_choose->setEnabled(true);
    ui.output_choose->setEnabled(true);

    update_progress(1.0);
    status(tr("finished!"));
}

void Convert::convert_start(void)
{
    update_progress(0.0);

    if(!libvlc_loaded)
    {
        if(convert_init() == 0)
            libvlc_loaded = true;
        else
        {
            status(tr("Internal error !"));
            return;
        }
    }

    ui.convert_button->setEnabled(false);
    ui.input_choose->setEnabled(false);
    ui.output_choose->setEnabled(false);

    status(tr("in progress"));

    thread = new ConvertThread(
         ui.input_file->text().toUtf8().data(),
        ui.output_file->text().toUtf8().data());

    connect(thread, SIGNAL(finished()), this, SLOT(finished()));
    connect(thread, SIGNAL(progress_changed(float)),
            this, SLOT(update_progress(float)));

    thread->start();
}

static bool convert_ready(const QString & in, const QString & out)
{
    if(in.isEmpty())
        return false;
    if(!out.endsWith("mkv"))
        return false;
    return true;
}

void Convert::output_choose(void)
{
    QString s = QFileDialog::getSaveFileName(this, tr("Open Output File"), "./",
            tr("Matroska Files (*.mkv)"));

    ui.convert_button->setEnabled(convert_ready(ui.input_file->text(), s));

    if (s.endsWith("mkv"))
    {
        status(NULL);
        ui.output_file->setText(s);
    }
    else
    {
        status(tr("Please select a file ending in .mkv"));
        ui.output_file->setText(NULL);
    }
}

void Convert::input_choose(void)
{
    QString s = QFileDialog::getOpenFileName(this, tr("Open Input File"), "./",
            tr("All Files (*.*)"));
    ui.input_file->setText(s);

    ui.convert_button->setEnabled(convert_ready(s, ui.output_file->text()));
}

void Convert::input_check(const QString & s)
{
    ui.convert_button->setEnabled(convert_ready(s, ui.output_file->text()));
}

void Convert::output_check(const QString & s)
{
    ui.convert_button->setEnabled(convert_ready(ui.input_file->text(), s));
}
