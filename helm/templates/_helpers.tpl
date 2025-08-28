{{/*
Expand the name of the chart.
*/}}
{{- define "uv-portal-desktop.name" -}}
uv-portal-desktop
{{- end }}

{{- define "uv-portal-desktop.db.name" -}}
uvportal-postgres
{{- end }}

{{- define "uv-portal-desktop.namespace" -}}
  {{- default .Release.Namespace .Values.namespace -}}
{{- end }}

{{- define "uv-portal-desktop.matchLabels" -}}
k8s-app: {{ include "uv-portal-desktop.name" . }}
app.kubernetes.io/name: {{ include "uv-portal-desktop.name" . }}
app.kubernetes.io/instance: {{ .Release.Name }}
{{- end -}}

{{- define "uv-portal-desktop.common.metaLabels" -}}
app.kubernetes.io/version: {{ .Chart.AppVersion }}
helm.sh/chart: {{ include "uv-portal-desktop.chart" . }}
app.kubernetes.io/managed-by: {{ .Release.Service }}
app.kubernetes.io/part-of: {{ include "uv-portal-desktop.name" . }}
{{- with .Values.commonMetaLabels}}
{{ toYaml . }}
{{- end }}
{{- end -}}

{{- define "uv-portal-desktop.labels" -}}
{{ include "uv-portal-desktop.matchLabels" . }}
{{ include "uv-portal-desktop.common.metaLabels" . }}
{{- end -}}

{{/*
Create a default fully qualified app name.
We truncate at 63 chars because some Kubernetes name fields are limited to this (by the DNS naming spec).
If release name contains chart name it will be used as a full name.
*/}}
{{- define "uv-portal-desktop.fullname" -}}
{{- if .Values.fullnameOverride }}
{{- .Values.fullnameOverride | trunc 63 | trimSuffix "-" }}
{{- else }}
{{- $name := default .Chart.Name .Values.nameOverride }}
{{- if contains $name .Release.Name }}
{{- .Release.Name | trunc 63 | trimSuffix "-" }}
{{- else }}
{{- printf "%s-%s" .Release.Name $name | trunc 63 | trimSuffix "-" }}
{{- end }}
{{- end }}
{{- end }}

{{/*
Create chart name and version as used by the chart label.
*/}}
{{- define "uv-portal-desktop.chart" -}}
{{- printf "%s-%s" .Chart.Name .Chart.Version | replace "+" "_" | trunc 63 | trimSuffix "-" }}
{{- end }}

{{/*
Create the name of the service account to use
*/}}
{{- define "uv-portal-desktop.serviceAccountName" -}}
{{- if .Values.serviceAccount.create }}
{{- default (include "uv-portal-desktop.fullname" .) .Values.serviceAccount.name }}
{{- else }}
{{- default "default" .Values.serviceAccount.name }}
{{- end }}
{{- end }}
