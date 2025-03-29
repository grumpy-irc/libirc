//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015 - 2019

#ifndef LIBMODE_H
#define LIBMODE_H

#include "serializableitem.h"

#define MODE_INCLUDE '+'
#define MODE_EXCLUDE '-'

namespace libirc
{
    class LIBIRCSHARED_EXPORT SingleMode : public SerializableItem
    {
        public:
            static QList<SingleMode> ToModeList(const QString &mode_string, QList<QString> parameters, const QList<char> &parameter_modes);

            SingleMode(QString mode);
            SingleMode(const QHash<QString, QVariant> &hash);
             ~SingleMode() override=default;
            bool IsIncluding();
            bool IsValid();
            QString ToString() const;
            char Get() const;
            void LoadHash(const QHash<QString, QVariant> &hash) override;
            QHash<QString, QVariant> ToHash() override;
            QString Parameter;

        protected:
            bool including;
            bool valid;
            char mode;
    };

    class LIBIRCSHARED_EXPORT Mode : public SerializableItem
    {
        public:
            Mode()=default;
            Mode(const QString &mode_string);
             ~Mode() override=default;
            /*!
             * \brief SetMode Take a string for a mode (like +xt) and try to apply it over the existing mode
             * \param mode_string String that represent some IRC mode
             * \param reset If true the modes after MODE_EXCLUDE sign will be reset, instead of append to exluding modes
             */
            void SetMode(const QString &mode_string, bool reset = false);
            bool Includes(char mode);
            bool Excludes(char mode);
            bool IsEmpty();
            void IncludeMode(char mode);
            void ExcludeMode(char mode);
            void ResetMode(char mode);
            void ResetModes(QList<char> modes);
            QList<char> GetExcluding();
            QList<char> GetIncluding();
            QString ToString();
            void LoadHash(const QHash<QString, QVariant> &hash) override;
            QHash<QString, QVariant> ToHash() override;
            QString Parameter;

        protected:
            QList<char> included_modes;
            QList<char> excluded_modes;
    };
}

#endif // MODE_H
