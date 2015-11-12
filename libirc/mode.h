//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

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
            static QList<SingleMode> ToModeList(QString mode_string, QList<QString> parameters, QList<char> parameter_modes);

            SingleMode(QString mode);
            virtual ~SingleMode();
            bool IsIncluding();
            char Get();
            void LoadHash(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash();
            QString Parameter;

        private:
            bool including;
            bool valid;
            char mode;
    };

    class LIBIRCSHARED_EXPORT Mode : public SerializableItem
    {
        public:
            Mode();
            Mode(QString mode_string);
            virtual ~Mode();
            /*!
             * \brief SetMode Take a string for a mode (like +xt) and try to apply it over the existing mode
             * \param mode_string String that represent some IRC mode
             * \param reset If true the modes after MODE_EXCLUDE sign will be reset, instead of append to exluding modes
             */
            void SetMode(QString mode_string, bool reset = false);
            bool Includes(char mode);
            bool Excludes(char mode);
            void IncludeMode(char mode);
            void ExcludeMode(char mode);
            void ResetMode(char mode);
            void ResetModes(QList<char> modes);
            QList<char> GetExcluding();
            QList<char> GetIncluding();
            QString ToString();
            void LoadHash(QHash<QString, QVariant> hash);
            QHash<QString, QVariant> ToHash();
            QString Parameter;

        protected:
            QList<char> included_modes;
            QList<char> excluded_modes;
    };
}

#endif // MODE_H
