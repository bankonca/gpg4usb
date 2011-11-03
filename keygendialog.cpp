/*
 *
 *      keygendialog.cpp
 *
 *      Copyright 2008 gpg4usb-team <gpg4usb@cpunk.de>
 *
 *      This file is part of gpg4usb.
 *
 *      Gpg4usb is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      Gpg4usb is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with gpg4usb.  If not, see <http://www.gnu.org/licenses/>
 */

#include "keygendialog.h"

KeyGenDialog::KeyGenDialog(GpgME::GpgContext *ctx, QWidget *parent)
 : QDialog(parent)
{
    mCtx = ctx;
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    this->setWindowTitle(tr("Generate Key"));
    this->setModal(true);
    generateKeyDialog();
}

void KeyGenDialog::generateKeyDialog()
{

    nameLabel = new QLabel(tr("Name:"));
    emailLabel = new QLabel(tr("E-Mailaddress::"));
    commentLabel = new QLabel(tr("Comment:"));
    keySizeLabel = new QLabel(tr("KeySize (in Bit):"));
    dateLabel = new QLabel(tr("Expiration Date:"));
    passwordLabel = new QLabel(tr("Password:"));
    repeatpwLabel = new QLabel(tr("Repeat Password:"));
    expireLabel = new QLabel(tr("Never Expire"));
    pwStrengthLabel = new QLabel(tr("Password: Strength\nWeak -> Strong"));
    errorLabel = new QLabel(tr(""));
    nameEdit = new QLineEdit(this);
    emailEdit = new QLineEdit(this);
    commentEdit = new QLineEdit(this);

    keySizeSpinBox = new QSpinBox(this);
    keySizeSpinBox->setRange(768, 8192);
    keySizeSpinBox->setValue(2048);

    keySizeSpinBox->setSingleStep(256);

    dateEdit = new QDateEdit(QDate::currentDate().addYears(5), this);
    dateEdit->setMinimumDate(QDate::currentDate());
    dateEdit->setDisplayFormat("dd/MM/yyyy");
    dateEdit->setCalendarPopup(true);
    dateEdit->setEnabled(false);

    expireCheckBox = new QCheckBox(this);
    expireCheckBox->setCheckState(Qt::Checked);

    passwordEdit = new QLineEdit(this);
    repeatpwEdit = new QLineEdit(this);

    passwordEdit->setEchoMode(QLineEdit::Password);
    repeatpwEdit->setEchoMode(QLineEdit::Password);

    pwStrengthSlider = new QSlider(this);
    pwStrengthSlider->setOrientation(Qt::Horizontal);
    pwStrengthSlider->setMaximum(6);
    pwStrengthSlider->setDisabled(true);
    pwStrengthSlider->setToolTip(tr("Password Strength"));
    pwStrengthSlider->setTickPosition(QSlider::TicksBelow);

    QGridLayout *vbox1 = new QGridLayout;
    vbox1->addWidget(nameLabel, 0, 0);
    vbox1->addWidget(nameEdit, 0, 1);
    vbox1->addWidget(emailLabel, 1, 0);
    vbox1->addWidget(emailEdit, 1, 1);
    vbox1->addWidget(commentLabel, 2, 0);
    vbox1->addWidget(commentEdit, 2, 1);
    vbox1->addWidget(dateLabel, 3, 0);
    vbox1->addWidget(dateEdit, 3, 1);
    vbox1->addWidget(expireCheckBox, 3, 2);
    vbox1->addWidget(expireLabel, 3, 3);
    vbox1->addWidget(keySizeLabel, 4, 0);
    vbox1->addWidget(keySizeSpinBox, 4, 1);
    vbox1->addWidget(passwordLabel, 5, 0);
    vbox1->addWidget(passwordEdit, 5, 1);
    vbox1->addWidget(pwStrengthLabel, 5, 3);
    vbox1->addWidget(repeatpwLabel, 6, 0);
    vbox1->addWidget(repeatpwEdit, 6, 1);
    vbox1->addWidget(pwStrengthSlider, 6, 3);

    QWidget *nameList = new QWidget(this);
    nameList->setLayout(vbox1);

    QVBoxLayout *vbox2 = new QVBoxLayout();
    vbox2->addWidget(nameList);
    vbox2->addWidget(errorLabel);
    vbox2->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(keyGenAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(expireCheckBox, SIGNAL(stateChanged(int)), this, SLOT(expireBoxChanged()));
    connect(passwordEdit, SIGNAL(textChanged(QString)), this, SLOT(passwordEditChanged()));
    this->setLayout(vbox2);
}

void KeyGenDialog::keyGenAccept()
{
    QString errorString = "";
    QString keyGenParams = "";
    /**
     * check for errors in keygen dialog input
     */
    if ((nameEdit->text()).size() < 5) {
        errorString.append(tr("  Name must contain at least five characters.  \n"));
    }
    if (passwordEdit->text() != repeatpwEdit->text()) {
        errorString.append(tr("  Password and Repeat don't match.  "));
    }

    if (errorString.isEmpty()) {
        /**
         * create the string for key generation
         */
        keyGenParams = "<GnupgKeyParms format=\"internal\">\n"
                       "Key-Type: DSA\n"
                       "Key-Length: 1024\n"
                       "Subkey-Type: ELG-E\n"
                       "Subkey-Length: "
                       + keySizeSpinBox->cleanText() + "\n"
                       "Name-Real: " + nameEdit->text() + "\n";
        if (!(commentEdit->text().isEmpty())) {
            keyGenParams += "Name-Comment: " + commentEdit->text() + "\n";
        }
        if (!(emailEdit->text().isEmpty())) {
            keyGenParams += "Name-Email: " + emailEdit->text() + "\n";
        }
        if (expireCheckBox->checkState()) {
            keyGenParams += "Expire-Date: 0\n";
        } else {
            keyGenParams += "Expire-Date: " + dateEdit->sectionText(QDateTimeEdit::YearSection) + "-" + dateEdit->sectionText(QDateTimeEdit::MonthSection) + "-" + dateEdit->sectionText(QDateTimeEdit::DaySection) + "\n";
        }
        if (!(passwordEdit->text().isEmpty())) {
            keyGenParams += "Passphrase: " + passwordEdit->text() + "\n";
        }
        keyGenParams += "</GnupgKeyParms>";

        KeyGenThread *kg = new KeyGenThread(keyGenParams, mCtx);
        kg->start();

        this->accept();

        QDialog *dialog = new QDialog(this, Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        dialog->setModal(true);
        dialog->setWindowTitle(tr("Generating Key..."));

        QLabel *waitMessage = new QLabel(tr("Collecting random data for key generation.\n This may take a while.\n To speed up the process use your computer\n (e.g. browse the net, listen to music,...)"));
        QProgressBar *pb = new QProgressBar();
        pb->setRange(0, 0);

        QVBoxLayout *layout = new QVBoxLayout(dialog);
        layout->addWidget(waitMessage);
        layout->addWidget(pb);
        dialog->setLayout(layout);

        dialog->show();

        while (kg->isRunning()) {
            QCoreApplication::processEvents();
        }

        dialog->close();
    } else {
        /**
         * create error message
         */
        errorLabel->setAutoFillBackground(true);
        QPalette error = errorLabel->palette();
        error.setColor(QPalette::Background, "#ff8080");
        errorLabel->setPalette(error);
        errorLabel->setText(errorString);

        this->show();
    }
}

void KeyGenDialog::expireBoxChanged()
{
    if (expireCheckBox->checkState()) {
        dateEdit->setEnabled(false);
    } else {
        dateEdit->setEnabled(true);
    }
}

void KeyGenDialog::passwordEditChanged()
{
    pwStrengthSlider->setValue(checkPassWordStrength());
    update();
}

int KeyGenDialog::checkPassWordStrength()
{
    int strength = 0;
    if ((passwordEdit->text()).length() > 7) {
        strength = strength + 2;
    }
    if ((passwordEdit->text()).contains(QRegExp("\\d"))) {
        strength++;
    }
    if ((passwordEdit->text()).contains(QRegExp("[a-z]"))) {
        strength++;
    }
    if ((passwordEdit->text()).contains(QRegExp("[A-Z]"))) {
        strength++;
    }
    if ((passwordEdit->text()).contains(QRegExp("\\W"))) {
        strength++;
    }

    return strength;
}
